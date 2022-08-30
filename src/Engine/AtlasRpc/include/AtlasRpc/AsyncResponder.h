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

    template<typename TRequest, typename TResponse>
    class AsyncResponder final : public IAsyncResponder
    {
    public:
        AsyncResponder(
            IAsyncResponderFactory& parentFactory,
            grpc::ServerCompletionQueue* completionQueue,
            BindMethod<TRequest, TResponse>& bind,
            InvokeMethod<TRequest, TResponse>& invoke)
                : m_Factory{parentFactory}
        , m_CompletionQueue{completionQueue}
        , m_Responder{m_Context}
        , m_BindCallback{bind}
        , m_InvokeCallback{invoke}
        {
        }

        void Bind() override
        {
            m_BindCallback(&m_Context, &m_Request, &m_Response, &m_CompletionQueue, &m_CompletionQueue, this);
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

            grpc::Status status = m_InvokeCallback(&m_Context, &m_Request, &m_Response);
            m_Responder.Finish(m_Response, status, this);
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
    };
}
