#include "AtlasRpcPCH.h"
#include "RpcManager.h"

#include <grpc++/server_builder.h>

#include "AsyncResponder.h"
#include "AsyncRpcService.h"
#include "AtlasTrace/Logging.h"


void atlas::rpc::RpcServer::Initialise(const uint16_t port)
{
    assert(!m_Server);

    grpc::ServerBuilder builder{};
    builder.AddListeningPort(std::format("127.0.0.1:{}", port), grpc::InsecureServerCredentials());

    for (const auto& service : m_Services)
    {
        grpc::Service* pService = service->GetService();
        if (!pService)
        {
            AT_ERROR(Rpc, "AsyncEndpoint GetService returned nullptr");
            continue;
        }

        builder.RegisterService(pService);
    }

    m_CompletionQueue = builder.AddCompletionQueue();
    m_Server = builder.BuildAndStart();

    m_RpcThread = std::thread{
        [this]
        {
            void* tag; // uniquely identifies a request.
            bool ok;

            std::vector<std::unique_ptr<IAsyncResponderFactory>> methods;
            for (const auto& service : m_Services)
            {
                service->GetEndpointFactories(methods);
            }

            for (const auto& factory : methods)
            {
                factory->Create(m_CompletionQueue.get())->Bind();
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
