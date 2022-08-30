#pragma once
#include "RpcManager.h"

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

        virtual std::vector<std::unique_ptr<IAsyncResponderFactory>> GetEndpointFactories() = 0;
    };
}
