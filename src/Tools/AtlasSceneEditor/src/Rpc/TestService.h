#pragma once

#include "Test.grpc.pb.h"
#include "AtlasRpc/AsyncRpcService.h"
#include "AtlasRpc/RpcManager.h"
#include "AtlasTrace/Logging.h"

namespace atlas::scene_editor::rpc
{
    class TestServiceImpl final : public EchoService::AsyncService, public atlas::rpc::IAsyncEndpoint
    {
    public:
        grpc::Service* GetService() override
        {
            return this;
        }

        void GetEndpointFactories(std::vector<std::unique_ptr<atlas::rpc::IAsyncResponderFactory>>& methods) override
        {
            methods.push_back(ATLAS_RPC_BIND_METHOD(SendEcho, Echo, EchoResponse));
            methods.push_back(ATLAS_RPC_BIND_METHOD(SendEchoReverse, Echo, EchoResponse));
        }

        ::grpc::Status SendEcho(::grpc::ServerContext* context, const ::Echo* request, ::EchoResponse* response) override
        {
            AT_INFO(Rpc, "SendEcho");
            return ::grpc::Status::OK;
        }
    };
}
