#include "AtlasGamePCH.h"
#include "SelectionSystem.h"

#include "imgui.h"
#include "InputId.h"
#include "SDL_keyboard.h"
#include "SDL_mouse.h"
#include "SDL_timer.h"
#include "SelectionComponent.h"
#include "AtlasInput/UserInputManager.h"
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
    if (ImGui::IsAnyItemHovered())
    {
        return;
    }

    using namespace controls;
    const auto user = input::UserInputManager::Get().GetUser(0);
    if (!user)
    {
        return;
    }

    if (user->IsButtonDown(input_id::c_buttonLeftTouch))
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
                m_PickRequests.emplace_back(m_PickingSystem.RequestPick(
                    user->GetAxisAbsoluteValue(input_id::c_axisYaw),
                    user->GetAxisAbsoluteValue(input_id::c_axisPitch)));
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
    using namespace controls;
    const auto user = input::UserInputManager::Get().GetUser(0);
    if (!user)
    {
        return;
    }

    if(!user->IsButtonDown(input_id::c_buttonExtendSelection))
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
