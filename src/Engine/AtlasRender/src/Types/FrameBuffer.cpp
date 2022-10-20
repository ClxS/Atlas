#include "AtlasRenderPCH.h"
#include "Types/FrameBuffer.h"

#include <cassert>

atlas::render::FrameBuffer::FrameBuffer()
{

}

atlas::render::FrameBuffer::~FrameBuffer()
{
    destroy(m_Handle);
}

void atlas::render::FrameBuffer::Initialise(
    const uint32_t width,
    const uint32_t height,
    const bool includeDepth,
    const bool wantsStencil,
    const bgfx::TextureFormat::Enum format,
    const uint64_t flags)
{
    m_Width = width;
    m_Height = height;
    m_IncludeDepth = includeDepth;
    m_WantsStencil = wantsStencil;
    m_Format = format;
    m_Flags = flags;

    if (isValid(m_Handle))
    {
        destroy(m_Handle);
    }

    const auto colour = createTexture2D(
        static_cast<uint16_t>(m_Width),
        static_cast<uint16_t>(m_Height),
        false,
        1,
        format,
        flags);

    if (includeDepth)
    {
        bgfx::TextureFormat::Enum depthFormat;
        if (wantsStencil)
        {
            assert(isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT_WRITE_ONLY));
            depthFormat = bgfx::TextureFormat::D24S8;
        }
        else
        {
            depthFormat =
                  isTextureValid(0, false, 1, bgfx::TextureFormat::D16,   BGFX_TEXTURE_RT_WRITE_ONLY) ? bgfx::TextureFormat::D16
                : isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT_WRITE_ONLY) ? bgfx::TextureFormat::D24S8
                : bgfx::TextureFormat::D32;
        }

        const auto depth = createTexture2D(
            static_cast<uint16_t>(m_Width),
            static_cast<uint16_t>(m_Height),
            false,
            1,
            depthFormat,
            BGFX_TEXTURE_RT_WRITE_ONLY);

        const bgfx::TextureHandle handles[] =
        {
            colour,
            depth
        };

        m_Handle = createFrameBuffer(BX_COUNTOF(handles), handles, true);
    }
    else
    {
        const bgfx::TextureHandle handles[] =
        {
            colour,
        };

        m_Handle = createFrameBuffer(BX_COUNTOF(handles), handles, true);
    }
}

void atlas::render::FrameBuffer::EnsureSize(const uint32_t width, const uint32_t height)
{
    if (width != m_Width || height != m_Height)
    {
        Initialise(width, height, m_IncludeDepth, m_WantsStencil, m_Format, m_Flags);
    }
}
