#include "AtlasGamePCH.h"
#include "SelectionRenderingSystem.h"

#include "ModelComponent.h"
#include "ModelRenderSystem.h"
#include "PostProcessSystem.h"
#include "SelectionComponent.h"
#include "TransformComponent.h"
#include "TransformPrivateComponent.h"

#include "AtlasAppHost/Application.h"
#include "AtlasRender/AssetRegistry.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/Debug/debugdraw.h"
#include "AtlasResource/ResourceLoader.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

using namespace atlas::game::scene::systems::rendering;
using namespace atlas::game::components;
using namespace interaction;
using namespace rendering;
using namespace atlas::render::resources::core_bundle;
using namespace bgfx;

namespace
{
    namespace local
    {
        struct VertexLayout
        {
            Eigen::Vector3f m_Position;
            Eigen::Vector2f m_Uv;
        };
    }
}

void SelectionRenderingSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    using namespace resource;
    using namespace render;
    using namespace resources;

    auto [width, height] = app_host::Application::Get().GetAppDimensions();
    m_Buffers.m_Silhouette.Initialise(width, height, true, true);
    m_Buffers.m_BlurredSilhouette.Initialise(width / 4, height / 4, false, false);

    setViewName(m_SilhouetteView, "Selection_Stencil");
    setViewName(m_BlurredView, "Selection_Blur");
    setViewName(m_OutlineFormationView, "Selection_FormOutline");
    setViewName(m_OutlineBlitView, "Selection_BlitOutline");

    setViewRect(m_SilhouetteView, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    setViewClear(m_SilhouetteView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);

    setViewClear(m_OutlineFormationView, BGFX_CLEAR_COLOR, 0x00000000);
    setViewMode(m_OutlineFormationView, ViewMode::Sequential);
    setViewRect(m_OutlineFormationView, 0, 0, BackbufferRatio::Equal);

    setViewClear(m_OutlineBlitView, BGFX_CLEAR_NONE);
    setViewMode(m_OutlineBlitView, ViewMode::Sequential);
    setViewRect(m_OutlineBlitView, 0, 0, BackbufferRatio::Equal);

    setViewClear(m_BlurredView, BGFX_CLEAR_NONE);
    setViewMode(m_BlurredView, ViewMode::Sequential);
    setViewRect(m_BlurredView, 0, 0, BackbufferRatio::Equal);

    setViewFrameBuffer(m_SilhouetteView, m_Buffers.m_Silhouette.GetHandle());
    setViewFrameBuffer(m_BlurredView, m_Buffers.m_BlurredSilhouette.GetHandle());
    setViewFrameBuffer(m_OutlineFormationView, m_Buffers.m_Silhouette.GetHandle());
    setViewFrameBuffer(m_OutlineBlitView, m_TargetFrameBuffer);

    m_Programs.m_StencilUpdate = ResourceLoader::LoadAsset<CoreBundle, ShaderProgram>(shaders::outline::c_outline_stencil);

    m_Programs.m_CreateBlur = ResourceLoader::LoadAsset<CoreBundle, ShaderProgram>(shaders::outline::c_outline_blur);
    m_Programs.m_Copy = ResourceLoader::LoadAsset<CoreBundle, ShaderProgram>(shaders::postprocess::c_copy);
    m_Programs.m_Clear = ResourceLoader::LoadAsset<CoreBundle, ShaderProgram>(shaders::outline::c_outline_clear);

    m_Samplers.m_Color = createUniform("s_texColor", UniformType::Sampler);
    static_assert(sizeof(local::VertexLayout) == sizeof(float) * 3 + sizeof(float) * 2);
    m_OutlineVertexLayout
        .begin()
        .add(Attrib::Position, 3, AttribType::Float)
        .add(Attrib::TexCoord0, 2, AttribType::Float)
        .end();

    constexpr float screenSpaceWidth = 1.0f;
    constexpr float screenSpaceHeight = 1.0f;
    constexpr float minX = -screenSpaceWidth;
    constexpr float maxX = screenSpaceWidth;
    constexpr float minY = -screenSpaceHeight;
    constexpr float maxy = screenSpaceHeight;
    constexpr float z = 0.0f;

    constexpr float minU = 0.0f;
    constexpr float maxU = 1.0f;
    float minV = 0.0f;
    float maxV = 1.0f;

    if (!getCaps()->originBottomLeft)
    {
        std::swap(minV, maxV);
    }

    local::VertexLayout vertices[6];
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

    static_assert(sizeof vertices == sizeof(local::VertexLayout) * 6);
    const Memory* vertexMemory = alloc(m_OutlineVertexLayout.getSize(6));
    std::memcpy(vertexMemory->data, vertices, m_OutlineVertexLayout.getSize(6));
    m_FullScreenQuad = createVertexBuffer(vertexMemory, m_OutlineVertexLayout);
}

void SelectionRenderingSystem::Render(atlas::scene::EcsManager& ecs)
{
    auto [width, height] = app_host::Application::Get().GetAppDimensions();
    m_Buffers.m_Silhouette.EnsureSize(width, height);
    m_Buffers.m_BlurredSilhouette.EnsureSize(width / 4, height / 4);

    setViewRect(m_SilhouetteView, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    setViewRect(m_OutlineFormationView, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    setViewRect(m_OutlineBlitView, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    touch(m_SilhouetteView);
    RenderSelectedObjects(
        ecs,
        1);
    RenderOutline();
    ApplyOutline();
}

void SelectionRenderingSystem::RenderOutline() const
{
    setVertexBuffer(0, m_FullScreenQuad.Get());
    setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);
    setTexture(0, m_Samplers.m_Color.Get(), getTexture(m_Buffers.m_Silhouette.GetHandle()));
    submit(m_BlurredView, m_Programs.m_CreateBlur->GetHandle());

    setVertexBuffer(0, m_FullScreenQuad.Get());
    setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);
    setStencil(BGFX_STENCIL_TEST_EQUAL
               | BGFX_STENCIL_FUNC_REF(0)
               | BGFX_STENCIL_FUNC_RMASK(0xff)
               | BGFX_STENCIL_OP_FAIL_S_KEEP
               | BGFX_STENCIL_OP_FAIL_Z_KEEP
               | BGFX_STENCIL_OP_PASS_Z_KEEP, BGFX_STENCIL_NONE);
    setTexture(0, m_Samplers.m_Color.Get(), getTexture(m_Buffers.m_BlurredSilhouette.GetHandle()));
    submit(m_OutlineFormationView, m_Programs.m_CreateBlur->GetHandle());
}

void SelectionRenderingSystem::ApplyOutline()
{
    setVertexBuffer(0, m_FullScreenQuad.Get());
    setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_ALWAYS | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
    setTexture(0, m_Samplers.m_Color.Get(), getTexture(m_Buffers.m_Silhouette.GetHandle()));
    submit(m_OutlineBlitView, m_Programs.m_Copy->GetHandle());
}

void SelectionRenderingSystem::RenderSelectedObjects(
    atlas::scene::EcsManager& ecs,
    const uint8_t stencilValue) const
{
    using namespace render::debug;

    for(auto [entity, model, positionRaw, position, _] : ecs.IterateEntityComponents<
            ModelComponent,
            TransformComponent,
            TransformPrivateComponent,
            SelectionComponent>())
    {
        if (!model.m_Model || !model.m_Model->GetMesh())
        {
            continue;
        }

        drawInstanced(
                m_SilhouetteView,
                model.m_Model,
                m_Programs.m_StencilUpdate,
                { position.m_Transform },
                BGFX_STATE_WRITE_RGB
                    | BGFX_STATE_WRITE_A
                    | BGFX_STATE_WRITE_Z
                    | BGFX_STATE_DEPTH_TEST_ALWAYS
                    | BGFX_STATE_CULL_CW,
                BGFX_STENCIL_TEST_ALWAYS
                    | BGFX_STENCIL_FUNC_REF(stencilValue)
                    | BGFX_STENCIL_FUNC_RMASK(0xff)
                    | BGFX_STENCIL_OP_FAIL_S_REPLACE
                    | BGFX_STENCIL_OP_FAIL_Z_REPLACE
                    | BGFX_STENCIL_OP_PASS_Z_REPLACE,
                BGFX_STENCIL_NONE,
                BGFX_DISCARD_ALL);

        for(auto& segment : model.m_Model->GetMesh()->GetSegments())
        {
            debug_draw::setWireframe(true);
            {
                bx::Vec3 centre =
                {
                    segment.m_Bounds.m_Sphere.center.x + positionRaw.m_Position.x(),
                    segment.m_Bounds.m_Sphere.center.y + positionRaw.m_Position.y(),
                    segment.m_Bounds.m_Sphere.center.z + positionRaw.m_Position.z(),
                };

                const bx::Sphere sphere = { { centre }, segment.m_Bounds.m_Sphere.radius };
                debug_draw::createScope();
                debug_draw::setColor(0x44ffc0f0_argb);
                debug_draw::setLod(0);
                debug_draw::draw(sphere);
            }
        }
    }
}
