#pragma once
#include "bgfx/bgfx.h"

namespace atlas::render
{
    template<typename THandle>
    class BgfxHandle
    {
    public:
        BgfxHandle() = default;

        // ReSharper disable once CppNonExplicitConvertingConstructor
        BgfxHandle(THandle handle)
            : m_Handle{handle}
        {
        }

        BgfxHandle& operator=(THandle handle)
        {
            if (isValid(m_Handle))
            {
                destroy(m_Handle);
            }

            m_Handle = handle;
            return *this;
        }

        BgfxHandle(const BgfxHandle&) = delete;
        BgfxHandle& operator=(const BgfxHandle&) = delete;

        BgfxHandle(BgfxHandle&&) = default;
        BgfxHandle& operator=(BgfxHandle&&) = default;

        ~BgfxHandle()
        {
            if (isValid(m_Handle))
            {
                destroy(m_Handle);
            }
        }

        [[nodiscard]] THandle Get() const { return m_Handle; }
    private:
        THandle m_Handle{BGFX_INVALID_HANDLE};
    };
}
