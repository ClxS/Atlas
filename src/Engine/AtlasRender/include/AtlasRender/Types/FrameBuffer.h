#pragma once
#include "bgfx/bgfx.h"
#include "bx/bx.h"

namespace atlas::render
{
    class FrameBuffer
    {
    public:
        FrameBuffer();
        ~FrameBuffer();

        void Initialise(
            uint32_t width,
            uint32_t height,
            bool includeDepth = true,
            bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA32F,
            uint64_t flags = BGFX_TEXTURE_RT);

        void EnsureSize(uint32_t width, uint32_t height);

        [[nodiscard]] bgfx::FrameBufferHandle GetHandle() const { return m_Handle; }

    private:
        uint32_t m_Width{};
        uint32_t m_Height{};
        bool m_IncludeDepth{false};
        bgfx::TextureFormat::Enum m_Format{bgfx::TextureFormat::RGBA32F};
        uint64_t m_Flags{0};

        bgfx::FrameBufferHandle m_Handle{BGFX_INVALID_HANDLE};
    };
}
