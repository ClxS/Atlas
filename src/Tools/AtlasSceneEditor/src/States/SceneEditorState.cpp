#include "AtlasSceneEditorPCH.h"
#include "SceneEditorState.h"

#include "AtlasGame/Scene/Components/Cameras/LookAtCameraComponent.h"
#include "AtlasGame/Scene/Components/Cameras/SphericalLookAtCameraComponent.h"
#include "AtlasGame/Scene/Components/Debug/DebugAxisComponent.h"
#include "AtlasGame/Scene/Components/Lighting/DirectionalLightComponent.h"
#include "AtlasGame/Scene/Systems/Debug/DebugAxisInputSystem.h"
#include "AtlasRender/Renderer.h"

namespace
{
    void addCameras(atlas::scene::EcsManager& ecs)
    {
        const auto cameraEntity = ecs.AddEntity();
        auto& camera2 = ecs.AddComponent<atlas::game::scene::components::cameras::LookAtCameraComponent>(cameraEntity);
        camera2.m_bIsRenderActive = false;
        camera2.m_bIsControlActive = false;
        camera2.m_Distance = 25.0f;
        camera2.m_LookAtPoint = {0.0f, 0.0f, 0.0f};
        camera2.m_Pitch = 30.0_degrees;
        camera2.m_Yaw = -135.0_degrees;
    }

    void addLights(atlas::scene::EcsManager& ecs)
    {
        const auto lightEntity = ecs.AddEntity();
        auto& light = ecs.AddComponent<atlas::game::scene::components::cameras::DirectionalLightComponent>(lightEntity);
        light.m_LightDirection = {0.08f, -0.5, -0.70f };
        light.m_LightColour = {1.0, 1.0, 1.0, 1.0f};

        light.m_LightDirection.normalize();
    }

    void addDebugRenderingComponents(atlas::scene::EcsManager& ecs)
    {
        const auto axisEntity = ecs.AddEntity();
        ecs.AddComponent<atlas::game::scene::components::debug::DebugAxisComponent>(axisEntity);
    }
}


void atlas::scene_editor::SceneEditorState::OnEntered(scene::SceneManager& sceneManager)
{
    scene::EcsManager& ecs = GetEcsManager();

    addCameras(ecs);
    addLights(ecs);
    addDebugRenderingComponents(ecs);

    EcsScene::OnEntered(sceneManager);
}

void atlas::scene_editor::SceneEditorState::ConstructSystems(scene::SystemsBuilder& simBuilder, scene::SystemsBuilder& frameBuilder)
{
    simBuilder.RegisterSystem<game::scene::systems::debug::DebugAxisInputSystem>();

    frameBuilder.RegisterSystem<game::scene::systems::debug::DebugAxisRenderSystem>();
}
