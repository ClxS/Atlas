#include "AtlasGamePCH.h"
#include "DebugAxisRenderSystem.h"

#include "AtlasCore/Colour.h"
#include "AtlasRender/Debug/DebugDraw.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "Debug/DebugAxisComponent.h"

void atlas::game::scene::systems::debug::DebugAxisRenderSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    SystemBase::Initialise(ecsManager);
}

void atlas::game::scene::systems::debug::DebugAxisRenderSystem::Update(atlas::scene::EcsManager& ecs)
{
    for(auto [_, debugAxis] : ecs.IterateEntityComponents<components::debug::DebugAxisComponent>())
    {
        render::debug::debug_draw::setColor(debugAxis.m_XAxisColour);
        render::debug::debug_draw::drawAxis(1.0f, 0.0f, 0.0f, 15, 1);

        render::debug::debug_draw::setColor(debugAxis.m_YAxisColour);
        render::debug::debug_draw::drawAxis(0.0f, 1.0f, 0.0f, 15, 1);

        render::debug::debug_draw::setColor(debugAxis.m_ZAxisColour);
        render::debug::debug_draw::drawAxis(0.0f, 0.0f, 1.0f, 15, 1);
    }
}
