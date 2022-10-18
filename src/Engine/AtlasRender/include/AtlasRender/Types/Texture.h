#pragma once
#include "bgfx/bgfx.h"
#include "bx/bx.h"

namespace atlas::render
{
    class Texture
    {
    public:
        Texture() = default;
        ~Texture();

        void Initialise(
            uint32_t width,
            uint32_t height,
            bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA32F,
            uint64_t flags = 0);

        void EnsureSize(uint32_t width, uint32_t height);

        [[nodiscard]] bgfx::TextureHandle GetHandle() const { return m_Handle; }

    private:
        uint32_t m_Width{};
        uint32_t m_Height{};
        bgfx::TextureFormat::Enum m_Format{bgfx::TextureFormat::RGBA32F};
        uint64_t m_Flags{0};

        bgfx::TextureHandle m_Handle{BGFX_INVALID_HANDLE};
    };
}
