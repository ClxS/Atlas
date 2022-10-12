#include "AtlasGamePCH.h"
#include "DebugAxisRenderSystem.h"

#include "DebugAxisComponent.h"
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
    render::debug::debug_draw::begin(m_View);

    for(auto [_, debugAxis] : ecs.IterateEntityComponents<components::debug::DebugAxisComponent>())
    {
        render::debug::debug_draw::setColor(debugAxis.m_XAxisColour);
        render::debug::debug_draw::drawAxis(1.0f, 0.0f, 0.0f, 15, 1);

        render::debug::debug_draw::setColor(debugAxis.m_YAxisColour);
        render::debug::debug_draw::drawAxis(0.0f, 1.0f, 0.0f, 15, 1);

        render::debug::debug_draw::setColor(debugAxis.m_ZAxisColour);
        render::debug::debug_draw::drawAxis(0.0f, 0.0f, 1.0f, 15, 1);
    }

    render::debug::debug_draw::setColor(core::colours::c_white);
    render::debug::debug_draw::drawGrid({ 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 100);
    render::debug::debug_draw::end();
}
