#include "AtlasGamePCH.h"
#include "ShadowMappingSystem.h"

#include "DirectionalLightComponent.h"
#include "AtlasRender/AssetRegistry.h"
#include "AtlasRender/Renderer.h"
#include "AtlasResource/ResourceLoader.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "bgfx/bgfx.h"

atlas::game::scene::systems::rendering::ShadowMappingSystem::ShadowMappingSystem(
    const bgfx::ViewId shadowViewId,
    const uint8_t shadowCasterRenderMask,
    const uint16_t shadowMapWidth,
    const uint16_t shadowMapHeight)
    : ModelRenderSystem
      {
          shadowViewId,
          std::vector { Pass {
              shadowViewId,
              shadowCasterRenderMask,
              0
              | BGFX_STATE_WRITE_RGB
              | BGFX_STATE_WRITE_A
              | BGFX_STATE_WRITE_Z
              | BGFX_STATE_DEPTH_TEST_LESS
              | BGFX_STATE_CULL_CCW
              | BGFX_STATE_MSAA,
              resource::ResourceLoader::LoadAsset<
                  render::resources::CoreBundle,
                  render::ShaderProgram>(render::resources::core_bundle::shaders::c_shadowMapBasic)
          }}
      }
      , m_ShadowViewId{shadowViewId}
      , m_ShadowMapWidth(shadowMapWidth)
      , m_ShadowMapHeight(shadowMapHeight)
{
}

void atlas::game::scene::systems::rendering::ShadowMappingSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    m_Programs.m_ShadowMap = resource::ResourceLoader::LoadAsset<
        render::resources::CoreBundle,
        render::ShaderProgram>(
        render::resources::core_bundle::shaders::c_shadowMapBasic);

    const bgfx::TextureHandle frameBufferTextures[] =
    {
        createTexture2D(
            m_ShadowMapWidth,
            m_ShadowMapHeight,
            false,
            1,
            bgfx::TextureFormat::D32F,
            BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL
        ),
    };

    m_ShadowMapFrameBuffer = createFrameBuffer(BX_COUNTOF(frameBufferTextures), frameBufferTextures, true);

    bgfx::setViewName(m_ShadowViewId, "ShadowMap");
    bgfx::setViewClear(m_ShadowViewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
    bgfx::setViewRect(m_ShadowViewId, 0, 0, m_ShadowMapWidth, m_ShadowMapHeight);
    setViewFrameBuffer(m_ShadowViewId, m_ShadowMapFrameBuffer);
}

void atlas::game::scene::systems::rendering::ShadowMappingSystem::Render(atlas::scene::EcsManager& ecs)
{
    const auto lightEntities = ecs.GetEntitiesWithComponents<components::lighting::DirectionalLightComponent>();
    if (lightEntities.empty())
    {
        bgfx::touch(m_ShadowViewId);
        render::setShadowCaster(0, {Eigen::Matrix4f::Identity(), getTexture(m_ShadowMapFrameBuffer)});
        return;
    }

    // c_lightDistance is sufficiently far away that we can consider it to be at infinity
    constexpr float c_lightDistance = 100.0f;
    const auto light = ecs.GetComponent<components::lighting::DirectionalLightComponent>(lightEntities[0]);
    Eigen::Matrix4f lightViewMatrix = maths_helpers::createLookAtMatrix(
        -light.m_Direction * c_lightDistance,
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f});

    Eigen::Matrix4f projectionMatrix = maths_helpers::createOrthographicMatrix(
        {-30.0f, 30.0f, -30.0f, 30.0f},
        -100.0f, c_lightDistance * 2.0f,
        0.0f,
        bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(
        m_ShadowViewId,
        lightViewMatrix.data(),
        projectionMatrix.data());

    const bgfx::Caps* caps = bgfx::getCaps();
    const float sy = caps->originBottomLeft ? 0.5f : -0.5f;
    const float sz = caps->homogeneousDepth ? 0.5f : 1.0f;
    const float tz = caps->homogeneousDepth ? 0.5f : 0.0f;

    Eigen::Matrix4f mtxCrop;
    mtxCrop.row(0) = Eigen::Vector4f(0.5f, 0.0f, 0.0f, 0.0f);
    mtxCrop.row(1) = Eigen::Vector4f(0.0f, sy, 0.0f, 0.0f);
    mtxCrop.row(2) = Eigen::Vector4f(0.0f, 0.0f, sz, 0.0f);
    mtxCrop.row(3) = Eigen::Vector4f(0.5f, 0.5f, tz, 1.0f);

    render::setShadowCaster(0, {projectionMatrix * lightViewMatrix, getTexture(m_ShadowMapFrameBuffer)});

    ModelRenderSystem::Render(ecs);
}
