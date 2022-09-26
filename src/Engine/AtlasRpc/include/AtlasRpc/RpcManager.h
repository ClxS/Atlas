#pragma once

#include <memory>
#include <string>
#include <vector>

#include <grpcpp/server.h>

#include "AsyncRpcService.h"
#include "MethodDefinitions.h"

namespace grpc
{
    class Service;
}

namespace atlas::rpc
{
#define ATLAS_RPC_BIND_METHOD(METHOD, REQUEST, RESPONSE) \
    std::make_unique<atlas::rpc::AsyncResponderFactory<REQUEST, RESPONSE>>(\
    [this](auto a, auto b, auto c, auto d, auto e, auto f) { Request##METHOD(a, b, c, d, e, f); },\
    [this](auto a, auto b, auto c) { return METHOD(a, b, c); })

    constexpr uint16_t c_DefaultRpcPort = 50051;

    class RpcServer
    {
    public:
        void Initialise(uint16_t port = c_DefaultRpcPort);
        RpcServer() = default;
        ~RpcServer();

        RpcServer(const RpcServer&) = delete;
        RpcServer& operator=(const RpcServer&) = delete;
        RpcServer(RpcServer&&) = delete;
        RpcServer& operator=(RpcServer&&) = delete;

        void RegisterService(std::unique_ptr<IAsyncEndpoint>&& service);

        template<typename TService>
        void RegisterService()
        {
            RegisterService(std::move(std::make_unique<TService>()));
        }

    private:
        std::vector<std::unique_ptr<IAsyncEndpoint>> m_Services;

        std::unique_ptr<grpc::Server> m_Server;
        std::unique_ptr<grpc::ServerCompletionQueue> m_CompletionQueue;
        std::thread m_RpcThread;
    };
}
