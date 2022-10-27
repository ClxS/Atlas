#include "AtlasGamePCH.h"
#include "ModelRenderSystem.h"

#include "ModelComponent.h"
#include "TransformComponent.h"
#include "TransformPrivateComponent.h"
#include "AtlasCore/MathsHelpers.h"
#include "AtlasRender/Renderer.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

atlas::game::scene::systems::rendering::ModelRenderSystem::ModelRenderSystem(
    const bgfx::ViewId view,
    std::vector<Pass>&& passes,
    const bool useInstancedRendering,
    std::function<void(atlas::scene::EntityId)> preRenderCallback)
    : m_View{view}
    , m_Passes{std::move(passes)}
    , m_UseInstancedRendering{useInstancedRendering}
    , m_PreRenderCallback{preRenderCallback}
{
}

void atlas::game::scene::systems::rendering::ModelRenderSystem::Render(atlas::scene::EcsManager& ecs)
{
    uint8_t renderMask = 0;
    for(const auto& pass : m_Passes)
    {
        bgfx::touch(pass.m_ViewId);
        renderMask |= pass.m_RenderMask;
    }

    for(auto [entity, model, position] : ecs.IterateEntityComponents<components::rendering::ModelComponent, components::TransformPrivateComponent>())
    {
        if ((model.m_RenderMask & renderMask) == 0 || !model.m_Model || !model.m_Model->GetMesh() || !model.m_Model->GetProgram())
        {
            continue;
        }

        assert(0 != (BGFX_CAPS_INSTANCING & bgfx::getCaps()->supported));

        constexpr uint16_t instanceStride = sizeof(Eigen::Matrix4f);

        constexpr uint32_t totalPositions = 1;
        const uint32_t numDrawableInstances = bgfx::getAvailInstanceDataBuffer(totalPositions, instanceStride);

        bgfx::InstanceDataBuffer idb{};
        allocInstanceDataBuffer(&idb, numDrawableInstances, instanceStride);

        bgfx::setTransform(position.m_Transform.data());

        for(const auto& pass : m_Passes)
        {
            if ((model.m_RenderMask & pass.m_RenderMask) == 0)
            {
                continue;
            }

            if (m_PreRenderCallback)
            {
                m_PreRenderCallback(entity);
            }

            if (m_UseInstancedRendering)
            {
                drawInstanced(
                    pass.m_ViewId,
                    model.m_Model,
                    pass.m_bOverrideProgram ? pass.m_bOverrideProgram : model.m_Model->GetProgram(),
                    { position.m_Transform },
                    pass.m_State,
                    BGFX_STENCIL_NONE,
                    BGFX_STENCIL_NONE,
                    BGFX_DISCARD_ALL);
            }
            else
            {
                draw(
                    pass.m_ViewId,
                    model.m_Model,
                    pass.m_bOverrideProgram ? pass.m_bOverrideProgram : model.m_Model->GetProgram(),
                    position.m_Transform,
                    pass.m_State,
                    BGFX_STENCIL_NONE,
                    BGFX_STENCIL_NONE,
                    BGFX_DISCARD_ALL);
            }
        }
    }
}
