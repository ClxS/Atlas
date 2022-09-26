#pragma once
#include "AsyncResponderFactory.h"

namespace atlas::rpc
{
    class IAsyncEndpoint
    {
    public:
        IAsyncEndpoint() = default;
        virtual ~IAsyncEndpoint() = default;
        IAsyncEndpoint(const IAsyncEndpoint&) = delete;
        IAsyncEndpoint(IAsyncEndpoint&&) = delete;
        IAsyncEndpoint& operator=(const IAsyncEndpoint&) = delete;
        IAsyncEndpoint& operator=(IAsyncEndpoint&&) = delete;

        virtual grpc::Service* GetService() = 0;

        virtual void GetEndpointFactories(std::vector<std::unique_ptr<IAsyncResponderFactory>>& methods) = 0;
    };
}
