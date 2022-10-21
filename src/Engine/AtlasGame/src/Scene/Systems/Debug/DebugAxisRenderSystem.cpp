#include "AtlasGamePCH.h"
#include "DebugAxisRenderSystem.h"

#include <numeric>

#include "DebugAxisComponent.h"
#include "SelectionComponent.h"
#include "TransformComponent.h"
#include "AtlasCore/Colour.h"
#include "AtlasRender/Debug/DebugDraw.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

atlas::game::scene::systems::debug::DebugAxisRenderSystem::DebugAxisRenderSystem(bgfx::ViewId view): m_View{view}
{
}

void atlas::game::scene::systems::debug::DebugAxisRenderSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    SystemBase::Initialise(ecsManager);
}

void atlas::game::scene::systems::debug::DebugAxisRenderSystem::Render(atlas::scene::EcsManager& ecs)
{
    render::debug::debug_draw::setColor(core::colours::c_white);
    render::debug::debug_draw::drawGrid({ 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 100);

    std::vector<Eigen::Vector3f> selectedPositions{};
    for(auto [entity, transform, _] : ecs.IterateEntityComponents<
            components::TransformComponent,
            components::interaction::SelectionComponent>())
    {
        selectedPositions.push_back(transform.m_Position);
    }

    if (selectedPositions.empty())
    {
        return;
    }

    const Eigen::Vector3f centrePoint = std::accumulate(
        selectedPositions.begin(),
        selectedPositions.end(),
        Eigen::Vector3f{0, 0, 0}) / selectedPositions.size();

    for(auto [_, debugAxis] : ecs.IterateEntityComponents<components::debug::DebugAxisComponent>())
    {
        render::debug::debug_draw::setDepthTestAlways();
        render::debug::debug_draw::drawAxis(centrePoint.x(), centrePoint.y(), centrePoint.z(), debugAxis.m_Length, debugAxis.m_Thickness);
    }

    render::debug::debug_draw::setDepthTestLess(true);
}
