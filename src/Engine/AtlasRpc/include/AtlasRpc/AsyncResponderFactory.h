#pragma once
#include "IAsyncResponder.h"

namespace grpc
{
    class ServerCompletionQueue;
}

namespace atlas::rpc
{
    class IAsyncResponderFactory
    {
    public:
        IAsyncResponderFactory() = default;
        virtual ~IAsyncResponderFactory() = default;
        IAsyncResponderFactory(const IAsyncResponderFactory&) = delete;
        IAsyncResponderFactory(IAsyncResponderFactory&&) = delete;
        IAsyncResponderFactory& operator=(const IAsyncResponderFactory&) = delete;
        IAsyncResponderFactory& operator=(IAsyncResponderFactory&&) = delete;

        virtual IAsyncResponder* Create(grpc::ServerCompletionQueue* completionQueue) = 0;
    };

    template<typename TRequest, typename TResponse>
    class AsyncResponderFactory : public IAsyncResponderFactory
    {
    public:
        AsyncResponderFactory(
            BindMethod<TRequest, TResponse> bind,
            InvokeMethod<TRequest, TResponse> invoke)
                : m_BindCallback{bind}
        , m_InvokeCallback{invoke}
        {
        }

        IAsyncResponder* Create(grpc::ServerCompletionQueue* completionQueue) override
        {
            return new AsyncResponder<TRequest, TResponse>(*this, completionQueue, m_BindCallback, m_InvokeCallback);
        }

    private:
        BindMethod<TRequest, TResponse> m_BindCallback;
        InvokeMethod<TRequest, TResponse> m_InvokeCallback;
    };
}
