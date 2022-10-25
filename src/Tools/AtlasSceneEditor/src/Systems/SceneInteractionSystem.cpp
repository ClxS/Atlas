#include "AtlasSceneEditorPCH.h"
#include "SceneInteractionSystem.h"

#include "AtlasGame/Components/SelectionComponent.h"
#include "AtlasInput/UserInputManager.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "Controls/InputId.h"

void atlas::scene_editor::SceneInteractionSystem::Update(scene::EcsManager& ecs)
{
    const auto user = input::UserInputManager::Get().GetUser(0);
    if (!user)
    {
        return;
    }

    if (user->IsButtonDown(input_id::c_buttonDelete))
    {
        const auto entities = ecs.GetEntitiesWithComponents<game::components::interaction::SelectionComponent>();
        for(const auto entity : entities)
        {
            ecs.RemoveEntity(entity);
        }
    }
}
