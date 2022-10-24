#include "AtlasGamePCH.h"
#include "TransformUpdateSystem.h"

#include "LookAtCameraComponent.h"
#include "TransformComponent.h"
#include "TransformPrivateComponent.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

void atlas::game::scene::systems::TransformUpdateSystem::Update(atlas::scene::EcsManager& ecs)
{
    for(auto [entity, transform, transformPrivate] : ecs.IterateEntityComponents<components::TransformComponent, components::TransformPrivateComponent>())
    {
        transformPrivate.m_Transform = maths_helpers::composeModelMatrix(transform.m_Position, transform.m_Scale, transform.m_Orientation);
    }
}
