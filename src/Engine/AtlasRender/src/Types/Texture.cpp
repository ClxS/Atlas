#include "AtlasRenderPCH.h"
#include "Texture.h"

atlas::render::Texture::~Texture()
{
    destroy(m_Handle);
}

void atlas::render::Texture::Initialise(
    const uint32_t width,
    const uint32_t height,
    const bgfx::TextureFormat::Enum format,
    const uint64_t flags)
{
    m_Width = width;
    m_Height = height;
    m_Format = format;
    m_Flags = flags;

    if (isValid(m_Handle))
    {
        destroy(m_Handle);
    }

    m_Handle = createTexture2D(
        static_cast<uint16_t>(m_Width),
        static_cast<uint16_t>(m_Height),
        false,
        1,
        format,
        flags);
}

void atlas::render::Texture::EnsureSize(const uint32_t width, const uint32_t height)
{
    if (width != m_Width || height != m_Height)
    {
        Initialise(width, height, m_Format, m_Flags);
    }
}
