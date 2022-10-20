#include "AtlasGamePCH.h"
#include "SelectionRenderingSystem.h"

#include "ModelComponent.h"
#include "ModelRenderSystem.h"
#include "PostProcessSystem.h"
#include "SelectionComponent.h"
#include "TransformPrivateComponent.h"

#include "AtlasAppHost/Application.h"
#include "AtlasRender/AssetRegistry.h"
#include "AtlasRender/Renderer.h"
#include "AtlasResource/ResourceLoader.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

namespace
{
    struct VertexLayout
    {
        Eigen::Vector3f m_Position;
        Eigen::Vector2f m_Uv;
    };
}

void atlas::game::scene::systems::rendering::SelectionRenderingSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    auto [width, height] = app_host::Application::Get().GetAppDimensions();
    m_Buffers.m_Silhouette.Initialise(width, height, true, true);
    m_Buffers.m_BlurredSilhouette.Initialise(width / 4, height / 4, false, false);

    bgfx::setViewName(m_SilhoutteView, "SelectionStencil");
    bgfx::setViewName(m_BlurredView, "SelectionBlur");
    bgfx::setViewName(m_OutlineBlitView + 0, "SelectionBlit0");
    bgfx::setViewName(m_OutlineBlitView + 1, "SelectionBlit1");

    bgfx::setViewRect(m_SilhoutteView, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    bgfx::setViewClear(m_SilhoutteView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);

    bgfx::setViewClear(m_OutlineBlitView + 0, BGFX_CLEAR_COLOR, 0x00000000);
    setViewMode(m_OutlineBlitView + 0, bgfx::ViewMode::Sequential);
    setViewRect(m_OutlineBlitView + 0, 0, 0, bgfx::BackbufferRatio::Equal);

    bgfx::setViewClear(m_OutlineBlitView + 1, BGFX_CLEAR_NONE);
    setViewMode(m_OutlineBlitView + 1, bgfx::ViewMode::Sequential);
    setViewRect(m_OutlineBlitView + 1, 0, 0, bgfx::BackbufferRatio::Equal);

    bgfx::setViewClear(m_BlurredView, BGFX_CLEAR_NONE);
    setViewMode(m_BlurredView, bgfx::ViewMode::Sequential);
    setViewRect(m_BlurredView, 0, 0, bgfx::BackbufferRatio::Equal);

    setViewFrameBuffer(m_SilhoutteView, m_Buffers.m_Silhouette.GetHandle());
    setViewFrameBuffer(m_BlurredView, m_Buffers.m_BlurredSilhouette.GetHandle());
    setViewFrameBuffer(m_OutlineBlitView + 0, m_Buffers.m_Silhouette.GetHandle());
    setViewFrameBuffer(m_OutlineBlitView + 1, m_TargetFrameBuffer);

    m_Programs.m_StencilUpdate = resource::ResourceLoader::LoadAsset<render::resources::CoreBundle,
                                                                render::ShaderProgram>(
        render::resources::core_bundle::shaders::outline::c_outline_stencil);

    m_Programs.m_CreateBlur = resource::ResourceLoader::LoadAsset<render::resources::CoreBundle,
                                                                render::ShaderProgram>(
        render::resources::core_bundle::shaders::outline::c_outline_blur);
    m_Programs.m_Copy = resource::ResourceLoader::LoadAsset<render::resources::CoreBundle,
                                                            render::ShaderProgram>(
                                                            render::resources::core_bundle::shaders::postprocess::c_copy);
    m_Programs.m_Clear = resource::ResourceLoader::LoadAsset<render::resources::CoreBundle,
                                                            render::ShaderProgram>(
        render::resources::core_bundle::shaders::outline::c_outline_clear);

    m_Samplers.m_Color = createUniform("s_texColor", bgfx::UniformType::Sampler);

    static_assert(sizeof(VertexLayout) == sizeof(float) * 3 + sizeof(float) * 2);
    m_OutlineVertexLayout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    const float screenSpaceWidth = 1.0f;
    const float screenSpaceHeight = 1.0f;
    const float minX = -screenSpaceWidth;
    const float maxX = screenSpaceWidth;
    const float minY = -screenSpaceHeight;
    const float maxy = screenSpaceHeight;
    constexpr float z = 0.0f;

    constexpr float minU = 0.0f;
    constexpr float maxU = 1.0f;
    float minV = 0.0f;
    float maxV = 1.0f;

    if (!bgfx::getCaps()->originBottomLeft)
    {
        std::swap(minV, maxV);
    }

    VertexLayout vertices[6];
    vertices[0].m_Position = { minX, minY, z };
    vertices[1].m_Position = { maxX, minY, z };
    vertices[2].m_Position = { maxX, maxy, z };
    vertices[3].m_Position = { minX, minY, z };
    vertices[4].m_Position = { maxX, maxy, z };
    vertices[5].m_Position = { minX, maxy, z };

    vertices[0].m_Uv = { minU, minV };
    vertices[1].m_Uv = { maxU, minV };
    vertices[2].m_Uv = { maxU, maxV };
    vertices[3].m_Uv = { minU, minV };
    vertices[4].m_Uv = { maxU, maxV };
    vertices[5].m_Uv = { minU, maxV };

    static_assert(sizeof(vertices) == sizeof(VertexLayout) * 6);
    const bgfx::Memory* vertexMemory = bgfx::alloc(m_OutlineVertexLayout.getSize(6));
    std::memcpy(vertexMemory->data, vertices, m_OutlineVertexLayout.getSize(6));
    m_FullScreenQuad = createVertexBuffer(vertexMemory, m_OutlineVertexLayout);
}

void atlas::game::scene::systems::rendering::SelectionRenderingSystem::Render(atlas::scene::EcsManager& ecs)
{
    auto [width, height] = app_host::Application::Get().GetAppDimensions();
    m_Buffers.m_Silhouette.EnsureSize(width, height);
    m_Buffers.m_BlurredSilhouette.EnsureSize(width / 4, height / 4);

    bgfx::setViewRect(m_SilhoutteView, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    bgfx::setViewRect(m_OutlineBlitView + 0, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    bgfx::setViewRect(m_OutlineBlitView + 1, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    bgfx::touch(m_SilhoutteView);
    RenderSelectedObjects(
        ecs,
        1);
    RenderOutline();
    ApplyOutline();
}

void atlas::game::scene::systems::rendering::SelectionRenderingSystem::RenderOutline() const
{
    setVertexBuffer(0, m_FullScreenQuad.Get());
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);
    setTexture(0, m_Samplers.m_Color.Get(), getTexture(m_Buffers.m_Silhouette.GetHandle()));
    submit(m_BlurredView, m_Programs.m_CreateBlur->GetHandle());

    setVertexBuffer(0, m_FullScreenQuad.Get());
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);
    bgfx::setStencil(BGFX_STENCIL_TEST_EQUAL
        | BGFX_STENCIL_FUNC_REF(0)
        | BGFX_STENCIL_FUNC_RMASK(0xff)
        | BGFX_STENCIL_OP_FAIL_S_KEEP
        | BGFX_STENCIL_OP_FAIL_Z_KEEP
        | BGFX_STENCIL_OP_PASS_Z_KEEP, BGFX_STENCIL_NONE);
    setTexture(0, m_Samplers.m_Color.Get(), getTexture(m_Buffers.m_BlurredSilhouette.GetHandle()));
    submit(m_OutlineBlitView + 0, m_Programs.m_CreateBlur->GetHandle());
}

void atlas::game::scene::systems::rendering::SelectionRenderingSystem::ApplyOutline()
{
    setVertexBuffer(0, m_FullScreenQuad.Get());
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_ALWAYS | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
    setTexture(0, m_Samplers.m_Color.Get(), getTexture(m_Buffers.m_Silhouette.GetHandle()));
    submit(m_OutlineBlitView + 1, m_Programs.m_Copy->GetHandle());
}

void atlas::game::scene::systems::rendering::SelectionRenderingSystem::RenderSelectedObjects(
    atlas::scene::EcsManager& ecs,
    const uint8_t stencilValue) const
{
    for(auto [entity, model, position, _] : ecs.IterateEntityComponents<
        components::rendering::ModelComponent,
        components::TransformPrivateComponent,
        components::interaction::SelectionComponent>())
    {
        if (!model.m_Model || !model.m_Model->GetMesh() || !model.m_Model->GetProgram())
        {
            continue;
        }

        assert(0 != (BGFX_CAPS_INSTANCING & bgfx::getCaps()->supported));
        constexpr uint16_t instanceStride = sizeof(Eigen::Matrix4f);
        constexpr uint32_t totalPositions = 1;
        const uint32_t numDrawableInstances = bgfx::getAvailInstanceDataBuffer(totalPositions, instanceStride);

        bgfx::InstanceDataBuffer idb{};
        allocInstanceDataBuffer(&idb, numDrawableInstances, instanceStride);

        bgfx::setState(
            BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_ALWAYS
            | BGFX_STATE_CULL_CW);
        bgfx::setStencil(
            BGFX_STENCIL_TEST_ALWAYS
            | BGFX_STENCIL_FUNC_REF(stencilValue)
            | BGFX_STENCIL_FUNC_RMASK(0xff)
            | BGFX_STENCIL_OP_FAIL_S_REPLACE
            | BGFX_STENCIL_OP_FAIL_Z_REPLACE
            | BGFX_STENCIL_OP_PASS_Z_REPLACE,
            BGFX_STENCIL_NONE);

        drawInstanced(
                m_SilhoutteView,
                model.m_Model,
                m_Programs.m_StencilUpdate,
                { position.m_Transform },
                BGFX_DISCARD_ALL);
    }
}
