#include "AtlasUIPCH.h"
#include "RmlRenderInterface.h"

#include "AssetRegistry.h"
#include "AtlasAppHost/Application.h"
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasRender/AssetTypes/TextureAsset.h"

atlas::ui::rml::RmlRenderInterface::RmlRenderInterface(bgfx::ViewId viewId)
    : m_Params{viewId}
{
    using namespace resources::registry;
    m_RmlVertexLayout
        .begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    m_TexturedProgram = resource::ResourceLoader::LoadAsset<CoreBundle, render::ShaderProgram>(core_bundle::shaders::c_rmlui_basic_textured);
    m_UntexturedProgram = resource::ResourceLoader::LoadAsset<CoreBundle, render::ShaderProgram>(core_bundle::shaders::c_rmlui_basic_untextured);
    m_Uniforms.m_Transform = createUniform("u_transform", bgfx::UniformType::Mat3);
    m_Uniforms.m_Sampler = createUniform("s_texColor", bgfx::UniformType::Sampler);
}

Rml::CompiledGeometryHandle atlas::ui::rml::RmlRenderInterface::CompileGeometry(Rml::Vertex* vertices, const int numVertices,
    int* indices, const int numIndices, Rml::TextureHandle texture)
{
    const bgfx::Memory* vertexMemory = bgfx::alloc(m_RmlVertexLayout.getSize(numVertices));
    const bgfx::Memory* indexMemory = bgfx::alloc(numIndices * sizeof(int));

    std::memcpy(vertexMemory->data, vertices, m_RmlVertexLayout.getSize(numVertices));
    std::memcpy(indexMemory->data, indices, numIndices * sizeof(int));

    const auto vertexBuffer = createVertexBuffer(vertexMemory, m_RmlVertexLayout);
    const auto indexBuffer = createIndexBuffer(indexMemory, BGFX_BUFFER_INDEX32);

    const auto handle = ++m_CompiledGeometryCounter;
    m_CompiledGeometry[handle] =
    {
        vertexBuffer,
        indexBuffer,
        texture
    };

    return handle;
}

void atlas::ui::rml::RmlRenderInterface::RenderCompiledGeometry(const Rml::CompiledGeometryHandle geometryHandle,
    const Rml::Vector2f& translation)
{
    const auto geometryIter = m_CompiledGeometry.find(geometryHandle);
    if (geometryIter == m_CompiledGeometry.end())
    {
        return;
    }

    const auto& geometry = *geometryIter;

    SetTransformUniform(translation);
    setVertexBuffer(0, geometry.second.m_VertexBuffer);
    setIndexBuffer(geometry.second.m_IndexBuffer);
    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)  // NOLINT(misc-redundant-expression)
    );

    if (geometry.second.m_Texture)
    {
        const auto textureIter = m_Textures.find(geometry.second.m_Texture);
        if (textureIter == m_Textures.end())
        {
            submit(m_Params.m_RenderViewId, m_UntexturedProgram->GetHandle());
        }
        else
        {
            setTexture(0, m_Uniforms.m_Sampler, textureIter->second.m_Handle);
            submit(m_Params.m_RenderViewId, m_TexturedProgram->GetHandle());
        }
    }
    else
    {
        submit(m_Params.m_RenderViewId, m_UntexturedProgram->GetHandle());
    }
}

void atlas::ui::rml::RmlRenderInterface::ReleaseCompiledGeometry(const Rml::CompiledGeometryHandle geometryHandle)
{
    const auto geometryIter = m_CompiledGeometry.find(geometryHandle);
    if (geometryIter == m_CompiledGeometry.end())
    {
        return;
    }

    const auto& geometry = *geometryIter;
    destroy(geometry.second.m_VertexBuffer);
    destroy(geometry.second.m_IndexBuffer);
    m_CompiledGeometry.erase(geometryIter);
}

void atlas::ui::rml::RmlRenderInterface::RenderGeometry(Rml::Vertex* vertices, const int numVertices, int* indices,
    const int numIndices, const Rml::TextureHandle texture, const Rml::Vector2f& translation)
{
    if (!m_TexturedProgram ||
        getAvailTransientVertexBuffer(numVertices, m_RmlVertexLayout) != static_cast<uint32_t>(numVertices) ||
        bgfx::getAvailTransientIndexBuffer(numIndices, true) != static_cast<uint32_t>(numIndices))
    {
        return;
    }

    bgfx::TransientVertexBuffer vb{};
    bgfx::TransientIndexBuffer ib{};

    allocTransientVertexBuffer(&vb, numVertices, m_RmlVertexLayout);
    allocTransientIndexBuffer(&ib, numIndices, true);

    std::memcpy(vb.data, vertices, m_RmlVertexLayout.getSize(numVertices));
    std::memcpy(ib.data, indices, numIndices * sizeof(int));

    SetTransformUniform(translation);
    setVertexBuffer(0, &vb);
    setIndexBuffer(&ib);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);  // NOLINT(misc-redundant-expression)

    if (texture)
    {
        const auto textureIter = m_Textures.find(texture);
        if (textureIter == m_Textures.end())
        {
            bgfx::submit(m_Params.m_RenderViewId, m_UntexturedProgram->GetHandle());
        }
        else
        {
            setTexture(0, m_Uniforms.m_Sampler, textureIter->second.m_Handle);
            bgfx::submit(m_Params.m_RenderViewId, m_TexturedProgram->GetHandle());
        }
    }
    else
    {
        bgfx::submit(m_Params.m_RenderViewId, m_UntexturedProgram->GetHandle());
    }
}

void atlas::ui::rml::RmlRenderInterface::SetScissorRegion(const int x, const int y, const int width, const int height)
{
    m_ScissorRegion = { x, y, width, height };
}

bool atlas::ui::rml::RmlRenderInterface::LoadTexture(Rml::TextureHandle& textureHandle, Rml::Vector2i& textureDimensions,
    const Rml::String& source)
{
    const auto id = resource::ResourceLoader::LookupId(source);
    if (!id.has_value())
    {
        return false;
    }

    const auto texture = resource::ResourceLoader::LoadAsset<render::TextureAsset>(id.value());
    if (!texture)
    {
        return false;
    }

    const auto handle = ++m_TextureCounter;
    m_Textures[handle] =
    {
        texture,
        texture->GetHandle(),
        texture->GetWidth(),
        texture->GetHeight()
    };

    textureDimensions = { static_cast<int>(texture->GetWidth()), static_cast<int>(texture->GetHeight()) };
    textureHandle = handle;
    return handle;
}

bool atlas::ui::rml::RmlRenderInterface::GenerateTexture(Rml::TextureHandle& textureHandle, const Rml::byte* source,
    const Rml::Vector2i& sourceDimensions)
{
    constexpr uint32_t c_stride = 4;
    const uint32_t size = c_stride * sourceDimensions.x * sourceDimensions.y;

    if (!isTextureValid(0, false, 1, bgfx::TextureFormat::RGBA8, 0) )
    {
        return false;
    }

    const bgfx::Memory* mem = bgfx::alloc(size);
    std::memcpy(mem->data, source, mem->size);

    const auto texHandle = createTexture2D(
        static_cast<uint16_t>(sourceDimensions.x),
        static_cast<uint16_t>(sourceDimensions.y),
        false,
        1,
        bgfx::TextureFormat::RGBA8,
        0,
        mem);

    if (!isValid(texHandle))
    {
        return false;
    }

    const auto handle = ++m_TextureCounter;
    m_Textures[handle] =
    {
        nullptr,
        texHandle,
        static_cast<uint32_t>(sourceDimensions.x),
        static_cast<uint32_t>(sourceDimensions.y)
    };

    textureHandle = handle;
    return handle;
}

void atlas::ui::rml::RmlRenderInterface::ReleaseTexture(const Rml::TextureHandle texture)
{
    const auto textureIter = m_Textures.find(texture);
    if (textureIter == m_Textures.end())
    {
        return;
    }

    if (!textureIter->second.m_Asset)
    {
        destroy(textureIter->second.m_Handle);
    }

    m_Textures.erase(textureIter);
}

void atlas::ui::rml::RmlRenderInterface::EnableScissorRegion(const bool enable)
{
    if (enable)
    {
        bgfx::setViewScissor(m_Params.m_RenderViewId,
                             static_cast<uint16_t>(m_ScissorRegion[0]),
                             static_cast<uint16_t>(m_ScissorRegion[1]),
                             static_cast<uint16_t>(m_ScissorRegion[2]),
                             static_cast<uint16_t>(m_ScissorRegion[3]));
    }
    else
    {
        bgfx::setViewScissor(m_Params.m_RenderViewId, 0, 0, 0, 0);
    }
}

void atlas::ui::rml::RmlRenderInterface::SetTransformUniform(const Rml::Vector2f& translation) const
{
    auto [width, height] = app_host::Application::Get().GetAppDimensions();
    Eigen::Vector4f frameBufferSize = { static_cast<float>(width), static_cast<float>(height), 0.0f, 0.0f };

    const Eigen::Affine2f t
    { Eigen::Translation2f(
        translation[0] - frameBufferSize.x() / 2.0f,
        translation[1] - frameBufferSize.y() / 2.0f) };
    const Eigen::Affine2f s { Eigen::Scaling((1.0f / frameBufferSize.x()) * 2.0f, (1.0f / frameBufferSize[1]) * 2.0f) };
    Eigen::Affine2f inversionScaling;
    if (!bgfx::getCaps()->originBottomLeft)
    {
        // RmlUI structures with the assumption that bottom left is 0,0. We need to manually flip if it is not
        inversionScaling = Eigen::Scaling(1.0f, -1.0f);
    }
    else
    {
        inversionScaling = Eigen::Scaling(1.0f, 1.0f);
    }

    Eigen::Matrix3f m = (inversionScaling * s * t).matrix();
    setUniform(m_Uniforms.m_Transform, m.data());
}
