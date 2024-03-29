﻿#include "AtlasGamePCH.h"
#include "AtlasGame/Scene/Systems/Rendering/PostProcessSystem.h"

#include "AtlasAppHost/Application.h"
#include "AtlasRender/AssetRegistry.h"
#include "AtlasResource/ResourceLoader.h"

namespace
{
    struct VertexLayout
    {
        Eigen::Vector3f m_Position;
        Eigen::Vector2f m_Uv;
    };

    [[nodiscard]] bgfx::VertexBufferHandle createFullscreenQuadVertexBuffer(
        const bool originBottomLeft,
        const float width = 1.0f,
        const float height = 1.0f)
    {
        bgfx::VertexLayout vertexLayout;
        vertexLayout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();

        const float minX = -width;
        const float maxX = width;
        const float minY = -height;
        const float maxy = height;

        constexpr float minU = 0.0f;
        constexpr float maxU = 1.0f;
        float minV = 0.0f;
        float maxV = 1.0f;

        constexpr float z = 0.0f;

        if (!originBottomLeft)
        {
            std::swap(minV, maxV);
        }

        VertexLayout vertices[6];
        vertices[0].m_Position = { minX, minY, z };
        vertices[0].m_Uv = { minU, minV };

        vertices[1].m_Position = { maxX, minY, z };
        vertices[1].m_Uv = { maxU, minV };

        vertices[2].m_Position = { maxX, maxy, z };
        vertices[2].m_Uv = { maxU, maxV };

        vertices[3].m_Position = { minX, minY, z };
        vertices[3].m_Uv = { minU, minV };

        vertices[4].m_Position = { maxX, maxy, z };
        vertices[4].m_Uv = { maxU, maxV };

        vertices[5].m_Position = { minX, maxy, z };
        vertices[5].m_Uv = { minU, maxV };

        static_assert(sizeof(vertices) == sizeof(VertexLayout) * 6);
        const bgfx::Memory* vertexMemory = bgfx::alloc(vertexLayout.getSize(6));
        std::memcpy(vertexMemory->data, vertices, vertexLayout.getSize(6));
        return createVertexBuffer(vertexMemory, vertexLayout);
    }

    constexpr std::array<const char*, 3> c_targets
    {
        {
            "FXAA",
            "Vignette",
            "Copy",
        }
    };
}

atlas::game::scene::systems::rendering::PostProcessSystem::PostProcessSystem(const bgfx::ViewId view, const bgfx::FrameBufferHandle* gbuffer)
    : m_View{view}
    , m_GBuffer{gbuffer}
{
}

atlas::game::scene::systems::rendering::PostProcessSystem::~PostProcessSystem()
{
}

void atlas::game::scene::systems::rendering::PostProcessSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    static_assert(sizeof(VertexLayout) == sizeof(float) * 3 + sizeof(float) * 2);
    m_PostProcessLayout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    const bgfx::RendererType::Enum renderer = bgfx::getRendererType();
    m_TexelHalf = bgfx::RendererType::Direct3D9 == renderer ? 0.5f : 0.0f;

    m_Programs.m_Copy = resource::ResourceLoader::LoadAsset<render::resources::CoreBundle,
                                                            render::ShaderProgram>(
        render::resources::core_bundle::shaders::postprocess::c_copy);
    m_Programs.m_Fxaa = resource::ResourceLoader::LoadAsset<render::resources::CoreBundle,
                                                            render::ShaderProgram>(
        render::resources::core_bundle::shaders::postprocess::c_fxaa);
    m_Programs.m_Vignette = resource::ResourceLoader::LoadAsset<render::resources::CoreBundle,
                                                                render::ShaderProgram>(
        render::resources::core_bundle::shaders::postprocess::c_vignette);

    m_Samplers.m_Color = createUniform("s_texColor", bgfx::UniformType::Sampler);
    m_Uniforms.m_FrameBufferSize = createUniform("frameBufferSize", bgfx::UniformType::Vec4);

    struct Target
    {
        std::string m_ViewName;
        render::BgfxHandle<bgfx::FrameBufferHandle> m_FrameBuffer;
    };

    for(uint16_t i = 0; i < static_cast<uint16_t>(c_targets.size()); ++i)
    {
        bgfx::setViewName(m_View + i, c_targets[i]);
        bgfx::setViewClear(m_View + i, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF);
        setViewMode(m_View + i, bgfx::ViewMode::Sequential);
        setViewRect(m_View + i, 0, 0, bgfx::BackbufferRatio::Equal);
    }

    m_FullScreenQuad = createFullscreenQuadVertexBuffer(bgfx::getCaps()->originBottomLeft);

    auto [width, height] = app_host::Application::Get().GetAppDimensions();
    m_Interstitials.m_FrameBuffer.Initialise(width, height);
}

void atlas::game::scene::systems::rendering::PostProcessSystem::Render(atlas::scene::EcsManager& ecs)
{
    PrepareFrame();

    bgfx::ViewId view = m_View;
    DoFxaa(view++, Scope::InputBuffer, Scope::Interstitial);
    DoVignette(view++, Scope::Interstitial, Scope::OutputBuffer);
    //DoCopy(view++, Scope::Interstitial, Scope::OutputBuffer);
}

bgfx::TextureHandle atlas::game::scene::systems::rendering::PostProcessSystem::GetInputAsTexture(const Scope target) const
{
    switch(target)
    {
    case Scope::Interstitial:
        return getTexture(m_Interstitials.m_FrameBuffer.GetHandle());
    case Scope::InputBuffer:
        return getTexture(*m_GBuffer);
    case Scope::OutputBuffer:
        break;
    }

    assert(false); // Shouldn't be here
    return BGFX_INVALID_HANDLE;
}

bgfx::FrameBufferHandle atlas::game::scene::systems::rendering::PostProcessSystem::GetTargetFrameBuffer(const Scope target) const
{
    switch(target)
    {
    case Scope::Interstitial:
        return m_Interstitials.m_FrameBuffer.GetHandle();
    case Scope::InputBuffer:
        return *m_GBuffer;
    case Scope::OutputBuffer:
        return BGFX_INVALID_HANDLE;
    }

    assert(false); // Shouldn't be here
    return BGFX_INVALID_HANDLE;
}

void atlas::game::scene::systems::rendering::PostProcessSystem::PrepareFrame()
{
    assert(m_Programs.m_Fxaa);
    assert(isValid(m_Programs.m_Fxaa->GetHandle()));

    auto [width, height] = app_host::Application::Get().GetAppDimensions();
    Eigen::Vector4f frameBufferSizeForUniform = { static_cast<float>(width), static_cast<float>(height), 0.0f, 0.0f };
    setUniform(m_Uniforms.m_FrameBufferSize.Get(), frameBufferSizeForUniform.data());

    m_Interstitials.m_FrameBuffer.EnsureSize(width, height);

    for(uint16_t i = 0; i < static_cast<uint16_t>(c_targets.size()); ++i)
    {
        setViewRect(m_View + i, 0, 0, bgfx::BackbufferRatio::Equal);
    }
}

void atlas::game::scene::systems::rendering::PostProcessSystem::DoFxaa(const bgfx::ViewId viewId, const Scope source, const Scope target) const
{
    setVertexBuffer(0, m_FullScreenQuad.Get());
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_MSAA);
    setViewFrameBuffer(viewId, GetTargetFrameBuffer(target));
    setTexture(0, m_Samplers.m_Color.Get(), GetInputAsTexture(source));
    submit(viewId, m_Programs.m_Fxaa->GetHandle());
}

void atlas::game::scene::systems::rendering::PostProcessSystem::DoVignette(const bgfx::ViewId viewId, const Scope source, const Scope target) const
{
    setVertexBuffer(0, m_FullScreenQuad.Get());
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_MSAA);
    setViewFrameBuffer(viewId, GetTargetFrameBuffer(target));
    setTexture(0, m_Samplers.m_Color.Get(), GetInputAsTexture(source));
    submit(viewId, m_Programs.m_Vignette->GetHandle());
}

void atlas::game::scene::systems::rendering::PostProcessSystem::DoCopy(const bgfx::ViewId viewId, const Scope source, const Scope target) const
{
    PerformCopy(viewId, GetInputAsTexture(source), GetTargetFrameBuffer(target));
}

void atlas::game::scene::systems::rendering::PostProcessSystem::PerformCopy(
    const bgfx::ViewId viewId,
    bgfx::TextureHandle source,
    bgfx::FrameBufferHandle target) const
{
    setVertexBuffer(0, m_FullScreenQuad.Get());
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_MSAA);
    setViewFrameBuffer(viewId, target);
    setTexture(0, m_Samplers.m_Color.Get(), source);
    submit(viewId, m_Programs.m_Copy->GetHandle());
}
