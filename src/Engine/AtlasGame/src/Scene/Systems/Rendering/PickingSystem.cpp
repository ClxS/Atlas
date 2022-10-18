#include "AtlasGamePCH.h"
#include "PickingSystem.h"

#include "AtlasRender/AssetRegistry.h"
#include "AtlasResource/ResourceLoader.h"
#include <bgfx/bgfx.h>

#include "ModelRenderSystem.h"
#include "bx/bx.h"

namespace
{
    Eigen::Vector4f decompose(const atlas::scene::EntityId entity)
    {
        const uint16_t r = entity.m_Value & 0x0000FFFF;
        const uint16_t g = entity.m_Value >> 16 & 0x0000FFFF;
        return { static_cast<float>(r), static_cast<float>(g), 0.0f, 1.0f };
    }
}

atlas::game::scene::systems::rendering::PickingSystem::PickingSystem(
    const bgfx::ViewId pickingSystemView,
    const uint8_t pickableRenderMask,
    const uint16_t pickFrameWidth,
    const uint16_t pickFrameHeight)
    : m_View{pickingSystemView}
    , m_PickableRenderMask{pickableRenderMask}
    , m_PickFrameWidth{pickFrameWidth}
    , m_PickFrameHeight{pickFrameHeight}
{

}

void atlas::game::scene::systems::rendering::PickingSystem::Initialise(atlas::scene::EcsManager& ecs)
{
    m_Programs.m_PickingObjectShader = resource::ResourceLoader::LoadAsset<
        render::resources::CoreBundle,
        render::ShaderProgram>(
        render::resources::core_bundle::shaders::c_shadowMapBasic);

    m_PickingFrame.Initialise(m_PickFrameWidth, m_PickFrameHeight, true);

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFFFFFFFF, 1.0f, 0);
    bgfx::setViewClear(m_View, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFFFFFFFF, 1.0f, 0);
    bgfx::setViewRect(m_View, 0, 0, m_PickFrameWidth, m_PickFrameHeight);
    setViewFrameBuffer(m_View, m_PickingFrame.GetHandle());

    m_ModelRenderSystem = std::make_unique<ModelRenderSystem>(m_View, std::vector {
        ModelRenderSystem::Pass {
            m_View,
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
            setUniform(m_Uniforms.m_ObjectId, decomposedValue.data(), 1);;
        });

    m_Uniforms.m_ObjectId = createUniform("u_id", bgfx::UniformType::Vec4);
}

void atlas::game::scene::systems::rendering::PickingSystem::Update(atlas::scene::EcsManager& ecs)
{
}

void atlas::game::scene::systems::rendering::PickingSystem::Render(atlas::scene::EcsManager& ecs)
{
    m_ModelRenderSystem->Render(ecs);
}
