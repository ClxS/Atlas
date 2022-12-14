#pragma once

#include <functional>
#include <grpcpp/server.h>

namespace atlas::rpc
{
    template<typename TRequest, typename TResponse>
    using BindMethod = std::function<void(grpc::ServerContext*, TRequest*, grpc::ServerAsyncResponseWriter<TResponse>*,
                                          grpc::CompletionQueue*, grpc::ServerCompletionQueue*, void*)>;

    template<typename TRequest, typename TResponse>
    using InvokeMethod = std::function<grpc::Status(grpc::ServerContext*, const TRequest*, TResponse*)>;

    template<typename TRequest, typename TResponse>
    using InvokeAsyncMethod = std::function<void(grpc::ServerContext*, const TRequest*, grpc::ServerAsyncResponseWriter<TResponse>*, void*)>;
}
