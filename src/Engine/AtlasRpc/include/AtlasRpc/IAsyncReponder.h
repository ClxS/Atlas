#pragma once

namespace atlas::rpc
{
    class IAsyncResponder
    {
    public:
        IAsyncResponder() = default;
        virtual ~IAsyncResponder() = default;
        IAsyncResponder(const IAsyncResponder&) = delete;
        IAsyncResponder(IAsyncResponder&&) = delete;
        IAsyncResponder& operator=(const IAsyncResponder&) = delete;
        IAsyncResponder& operator=(IAsyncResponder&&) = delete;

        virtual void Bind() = 0;
        virtual void Process() = 0;
    };
}
