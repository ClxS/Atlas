#pragma once
#include "AsyncResponder.h"
#include "IAsyncResponderFactory.h"
#include "MethodDefinitions.h"

namespace grpc
{
    class ServerCompletionQueue;
}

namespace atlas::rpc
{
    template<typename TRequest, typename TResponse>
    class AsyncResponderFactory final : public IAsyncResponderFactory
    {
    public:
        AsyncResponderFactory(
            BindMethod<TRequest, TResponse> bind,
            InvokeMethod<TRequest, TResponse> invoke,
            InvokeAsyncMethod<TRequest, TResponse> invokeAsync)
                : m_BindCallback{bind}
                , m_InvokeCallback{invoke}
                , m_InvokeAsyncCallback{invokeAsync}
        {
        }

        IAsyncResponder* Create(grpc::ServerCompletionQueue* completionQueue) override
        {
            return new AsyncResponder<TRequest, TResponse>(*this, completionQueue, m_BindCallback, m_InvokeCallback, m_InvokeAsyncCallback);
        }

    private:
        BindMethod<TRequest, TResponse> m_BindCallback;
        InvokeMethod<TRequest, TResponse> m_InvokeCallback;
        InvokeAsyncMethod<TRequest, TResponse> m_InvokeAsyncCallback;
    };
}
