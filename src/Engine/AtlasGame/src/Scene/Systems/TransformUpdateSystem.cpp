#include "AtlasGamePCH.h"
#include "TransformUpdateSystem.h"

#include "TransformComponent.h"
#include "TransformPrivateComponent.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

void atlas::game::scene::systems::TransformUpdateSystem::Update(atlas::scene::EcsManager& ecs)
{
    for(auto [entity, transform, transformPrivate] : ecs.IterateEntityComponents<components::TransformComponent, components::TransformPrivateComponent>())
    {
        Eigen::Affine3f t{Eigen::Translation3f(transform.m_Position.cast<float>())};
        transformPrivate.m_Transform = (transform.m_Orientation * t).matrix();
    }
}
