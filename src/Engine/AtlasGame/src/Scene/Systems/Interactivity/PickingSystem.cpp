#include "AtlasGamePCH.h"
#include "PickingSystem.h"

#include <bgfx/bgfx.h>
#include "AtlasRender/AssetRegistry.h"
#include "AtlasResource/ResourceLoader.h"

#include "ModelRenderSystem.h"
#include "SDL_mouse.h"
#include "AtlasAppHost/Application.h"
#include "AtlasTrace/Logging.h"

namespace
{
    constexpr uint32_t c_stride = 8;
    constexpr uint32_t c_mask = (1 << c_stride) - 1;

    Eigen::Vector4f decompose(const atlas::scene::EntityId entity)
    {
        const uint16_t r = entity.m_Value & c_mask;
        const uint16_t g = (entity.m_Value >> (32 - c_stride * 1)) & c_mask;
        const uint16_t b = (entity.m_Value >> (32 - c_stride * 2)) & c_mask;
        return { static_cast<float>(r) / c_mask, static_cast<float>(g) / c_mask, static_cast<float>(b) / c_mask, 1.0f };
    }

    atlas::scene::EntityId compose(const uint8_t r, const uint8_t g, const uint8_t b)
    {
        if (r == 0xFF && g == 0xFF && b == 0xFF)
        {
            return atlas::scene::EntityId::Invalid();
        }

        return atlas::scene::EntityId {
                static_cast<int32_t>(r) |
                static_cast<int32_t>(g) << c_stride * 1ULL |
                static_cast<int32_t>(b) << c_stride * 1ULL };
    }
}

atlas::game::scene::systems::interacivity::PickingSystem::PickingSystem(
    const bgfx::ViewId pickingBufferView,
    const bgfx::ViewId pickingBlitView,
    const uint8_t pickableRenderMask,
    const uint16_t pickFrameWidth,
    const uint16_t pickFrameHeight)
    : m_PickingBufferView{pickingBufferView}
    , m_PickingBlitView{pickingBlitView}
    , m_PickableRenderMask{pickableRenderMask}
    , m_PickFrameWidth{pickFrameWidth}
    , m_PickFrameHeight{pickFrameHeight}
{
}

void atlas::game::scene::systems::interacivity::PickingSystem::Initialise(atlas::scene::EcsManager& ecs)
{
    m_Programs.m_PickingObjectShader = resource::ResourceLoader::LoadAsset<
        render::resources::CoreBundle,
        render::ShaderProgram>(
        render::resources::core_bundle::shaders::c_shadowMapBasic);

    m_PickingFrame.Initialise(
        m_PickFrameWidth,
        m_PickFrameHeight,
        true,
        bgfx::TextureFormat::RGBA8,
        0
            | BGFX_TEXTURE_RT
            | BGFX_SAMPLER_MIN_POINT
            | BGFX_SAMPLER_MAG_POINT
            | BGFX_SAMPLER_MIP_POINT
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP);
    m_BlittingFrame.Initialise(
        m_PickFrameWidth,
        m_PickFrameHeight,
        bgfx::TextureFormat::RGBA8,
        0
            | BGFX_TEXTURE_BLIT_DST
            | BGFX_TEXTURE_READ_BACK
            | BGFX_SAMPLER_MIN_POINT
            | BGFX_SAMPLER_MAG_POINT
            | BGFX_SAMPLER_MIP_POINT
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
        );

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFFFFFFFF, 1.0f, 0);
    bgfx::setViewClear(m_PickingBufferView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFFFFFFFF, 1.0f, 0);
    bgfx::setViewRect(m_PickingBufferView, 0, 0, m_PickFrameWidth, m_PickFrameHeight);
    setViewFrameBuffer(m_PickingBufferView, m_PickingFrame.GetHandle());

    m_Uniforms.m_ObjectId = createUniform("u_entityId", bgfx::UniformType::Vec4);
    m_ModelRenderSystem = std::make_unique<rendering::ModelRenderSystem>(m_PickingBufferView, std::vector {
        rendering::ModelRenderSystem::Pass {
            m_PickingBufferView,
            m_PickableRenderMask,
            BGFX_STATE_DEFAULT,
            resource::ResourceLoader::LoadAsset<
              render::resources::CoreBundle,
              render::ShaderProgram>(render::resources::core_bundle::shaders::c_picking)
        }},
        false,
        [this](atlas::scene::EntityId entity)
        {
            Eigen::Vector4f decomposedValue = decompose(entity);
            setUniform(m_Uniforms.m_ObjectId, decomposedValue.data());
        });

    m_PickingFrameData.resize(m_PickFrameWidth * m_PickFrameHeight * 4);
}

void atlas::game::scene::systems::interacivity::PickingSystem::Update(atlas::scene::EcsManager& ecs)
{
}

auto atlas::game::scene::systems::interacivity::PickingSystem::Render(atlas::scene::EcsManager& ecs) -> void
{
    m_ModelRenderSystem->Render(ecs);

    const bgfx::TextureHandle blitTexture = m_BlittingFrame.GetHandle();
    blit(m_PickingBlitView, blitTexture, 0, 0, getTexture(m_PickingFrame.GetHandle()));
    readTexture(blitTexture, m_PickingFrameData.data());

    auto [appWidth, appHeight] = app_host::Application::Get().GetAppDimensions();
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    float xfrac = static_cast<float>(mouseX) / appWidth;
    float yfrac = static_cast<float>(mouseY) / appHeight;
    int dataPixelX = static_cast<int>(xfrac * m_PickFrameWidth);
    int dataPixelY = static_cast<int>(yfrac * m_PickFrameHeight);

    if (dataPixelX >= 0 && dataPixelY >= 0 && dataPixelX < m_PickFrameWidth && dataPixelY < m_PickFrameHeight)
    {
        auto pixelData = (dataPixelY * m_PickFrameWidth + dataPixelX) * 4;
        const atlas::scene::EntityId entity = compose(
            m_PickingFrameData[pixelData + 0],
            m_PickingFrameData[pixelData + 1],
            m_PickingFrameData[pixelData + 2]);
    }
}
