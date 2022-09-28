#pragma once

#include "IAsyncReponder.h"
#include "IAsyncResponderFactory.h"
#include "MethodDefinitions.h"

namespace atlas::rpc
{
    template<typename TRequest, typename TResponse>
    class AsyncResponder final : public IAsyncResponder
    {
    public:
        AsyncResponder(
            IAsyncResponderFactory& parentFactory,
            grpc::ServerCompletionQueue* completionQueue,
            BindMethod<TRequest, TResponse>& bind,
            InvokeMethod<TRequest, TResponse>& invoke,
            InvokeAsyncMethod<TRequest, TResponse>& invokeAsync)
                : m_Factory{parentFactory}
                , m_CompletionQueue{completionQueue}
                , m_Responder{&m_Context}
                , m_BindCallback{bind}
                , m_InvokeCallback{invoke}
                , m_InvokeAsyncCallback{invokeAsync}
        {
        }

        void Bind() override
        {
            m_BindCallback(&m_Context, &m_Request, &m_Responder, m_CompletionQueue, m_CompletionQueue, this);
        }

        void Process() override
        {
            if (m_bIsInvoked)
            {
                delete this;
                return;
            }

            // Create a new Responder to take the place of this one.
            m_Factory.Create(m_CompletionQueue)->Bind();

            if (m_InvokeAsyncCallback)
            {
                m_InvokeAsyncCallback(&m_Context, &m_Request, &m_Responder, this);
            }
            else if (m_InvokeCallback)
            {
                grpc::Status status = m_InvokeCallback(&m_Context, &m_Request, &m_Response);
                m_Responder.Finish(m_Response, status, this);
            }
            else
            {
                m_Responder.Finish(m_Response, grpc::Status(grpc::StatusCode::UNIMPLEMENTED, ""), this);
            }

            m_bIsInvoked = true;
        }

    private:
        bool m_bIsInvoked = false;

        IAsyncResponderFactory& m_Factory;
        grpc::ServerContext m_Context;
        grpc::ServerCompletionQueue* m_CompletionQueue;
        grpc::ServerAsyncResponseWriter<TResponse> m_Responder;

        TRequest m_Request;
        TResponse m_Response;

        BindMethod<TRequest, TResponse>& m_BindCallback;
        InvokeMethod<TRequest, TResponse>& m_InvokeCallback;
        InvokeAsyncMethod<TRequest, TResponse>& m_InvokeAsyncCallback;
    };
}
