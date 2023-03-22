#pragma once
#include <bgfx/bgfx.h>

namespace atlas::game::utility
{
    class ViewTransformCache
    {
    public:
        struct ViewTransform
        {
            Eigen::Matrix4f m_View;
            Eigen::Matrix4f m_Projection;
        };

        static void SetViewTransform(const bgfx::ViewId viewId, Eigen::Matrix4f view, Eigen::Matrix4f projection)
        {
            assert(viewId < 256);
            m_Views[viewId] = ViewTransform { std::move(view), std::move(projection) };
        }

        static const ViewTransform& GetViewTransform(const bgfx::ViewId viewId)
        {
            return m_Views[viewId];
        }

    private:
        inline static ViewTransform m_Views[256];
    };
}
