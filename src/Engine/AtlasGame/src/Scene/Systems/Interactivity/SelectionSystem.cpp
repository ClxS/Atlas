#include "AtlasGamePCH.h"
#include "SelectionSystem.h"

#include "SDL_keyboard.h"
#include "SDL_mouse.h"
#include "SDL_timer.h"
#include "SelectionComponent.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

namespace
{
    constexpr uint32_t c_clickTimeThreshold = 200;
}

void atlas::game::scene::systems::interactivity::SelectionSystem::Initialise(atlas::scene::EcsManager& ecs)
{

}

void atlas::game::scene::systems::interactivity::SelectionSystem::Update(atlas::scene::EcsManager& ecs)
{
    int mouseX, mouseY;
    const uint32_t buttons = SDL_GetMouseState(&mouseX, &mouseY);
    if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0)
    {
        if (!m_IsMouseDown)
        {
            m_IsMouseDown = true;
            m_MouseDownTime = SDL_GetTicks();
        }
    }
    else
    {
        if (m_IsMouseDown)
        {
            m_IsMouseDown = false;
            const uint32_t currentTime = SDL_GetTicks();
            if (currentTime - m_MouseDownTime < c_clickTimeThreshold)
            {
                m_PickRequests.emplace_back(m_PickingSystem.RequestPick(mouseX, mouseY));
            }
        }
    }

    for(auto& request : m_PickRequests)
    {
        if (request.m_Request.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
        {
            continue;
        }

        ConsumePick(ecs, request);
        request.m_IsComplete = true;
    }

    std::erase_if(
        m_PickRequests,
        [](PickRequest const& task)
        {
            return task.m_IsComplete;
        });
}

void atlas::game::scene::systems::interactivity::SelectionSystem::ConsumePick(atlas::scene::EcsManager& ecs, PickRequest& value)
{
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
    if(!keyboardState[SDL_SCANCODE_LCTRL] && !keyboardState[SDL_SCANCODE_RCTRL])
    {
        for(auto [entity, _] : ecs.IterateEntityComponents<components::interaction::SelectionComponent>())
        {
            ecs.RemoveComponent<components::interaction::SelectionComponent>(entity);
        }
    }

    const atlas::scene::EntityId pickedEntity = value.m_Request.get();
    if (pickedEntity.IsValid())
    {
        ecs.AddComponent<components::interaction::SelectionComponent>(pickedEntity);
    }
}
