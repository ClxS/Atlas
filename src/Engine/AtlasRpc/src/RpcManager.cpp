#include "AtlasRpcPCH.h"
#include "RpcManager.h"

#include <grpc++/server_builder.h>

#include "AsyncRpcService.h"
#include "AtlasTrace/Logging.h"


void atlas::rpc::RpcServer::Initialise(const uint16_t port)
{
    assert(!m_Server);

    grpc::ServerBuilder builder{};
    builder.AddListeningPort(std::format("0.0.0.0:{}", port), grpc::InsecureServerCredentials());

    for (const auto& service : m_Services)
    {
        builder.RegisterService(reinterpret_cast<grpc::Service*>(service.get()));
    }

    m_CompletionQueue = builder.AddCompletionQueue();
    m_Server = builder.BuildAndStart();

    m_RpcThread = std::thread{
        [this]()
        {
            void* tag; // uniquely identifies a request.
            bool ok;

            for (const auto& service : m_Services)
            {
                for(const auto& factory : service->GetEndpointFactories())
                {
                    factory->Create(m_CompletionQueue.get())->Bind();
                }
            }

            while (true)
            {
                if (!m_CompletionQueue->Next(&tag, &ok))
                {
                    AT_INFO(AtlasRpc, "Server queue exhausted, exiting RPC thread.");
                    return;
                }

                if (!ok)
                {
                    AT_ERROR(AtlasRpc, "CompletionQueue::Next returned false OK flag");
                    return;
                }

                static_cast<IAsyncResponder*>(tag)->Process();
            }
        }
    };
}

atlas::rpc::RpcServer::~RpcServer()
{
}

void atlas::rpc::RpcServer::RegisterService(std::unique_ptr<IAsyncEndpoint>&& service)
{
    m_Services.emplace_back(std::move(service));
}
