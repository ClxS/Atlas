#pragma once
#include <grpcpp/server.h>

namespace atlas::rpc
{
    class IAsyncResponderFactory
    {
    public:
        virtual ~IAsyncResponderFactory() = default;

        virtual IAsyncResponder* Create(grpc::ServerCompletionQueue* completionQueue) = 0;
    };
}
