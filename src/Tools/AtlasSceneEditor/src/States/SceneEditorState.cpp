#include "AtlasSceneEditorPCH.h"
#include "SceneEditorState.h"

#include "Constants.h"
#include "AtlasAppHost/Application.h"
#include "AtlasGame/Components/DebugAxisComponent.h"
#include "AtlasGame/Components/DirectionalLightComponent.h"
#include "AtlasGame/Components/LookAtCameraComponent.h"
#include "AtlasGame/Scene/Systems/Cameras/CameraControllerSystem.h"
#include "AtlasGame/Scene/Systems/Cameras/CameraViewProjectionUpdateSystem.h"
#include "AtlasGame/Scene/Systems/Debug/DebugAxisInputSystem.h"
#include "AtlasGame/Scene/Systems/Rendering/PostProcessSystem.h"

namespace
{
    void addCameras(atlas::scene::EcsManager& ecs)
    {
        const auto cameraEntity = ecs.AddEntity();
        auto& camera2 = ecs.AddComponent<atlas::game::components::cameras::LookAtCameraComponent>(cameraEntity);
        camera2.m_IsRenderActive = true;
        camera2.m_IsControlActive = true;
        camera2.m_Distance = 25.0f;
        camera2.m_LookAtPoint = {0.0f, 0.0f, 0.0f};
        camera2.m_Pitch = 30.0_degrees;
        camera2.m_Yaw = -135.0_degrees;
    }

    void addLights(atlas::scene::EcsManager& ecs)
    {
        const auto lightEntity = ecs.AddEntity();
        auto& light = ecs.AddComponent<atlas::game::components::lighting::DirectionalLightComponent>(lightEntity);
        light.m_Direction = {0.08f, -0.5, -0.70f };
        light.m_Colour = 0xffffffff_argb;

        light.m_Direction.normalize();
    }

    void addDebugRenderingComponents(atlas::scene::EcsManager& ecs)
    {
        const auto axisEntity = ecs.AddEntity();
        ecs.AddComponent<atlas::game::components::debug::DebugAxisComponent>(axisEntity);
    }
}


void atlas::scene_editor::SceneEditorState::OnEntered(scene::SceneManager& sceneManager)
{
    ClearScene();
    EcsScene::OnEntered(sceneManager);
}

void atlas::scene_editor::SceneEditorState::ConstructSystems(scene::SystemsBuilder& simBuilder, scene::SystemsBuilder& frameBuilder)
{
    // Game
    {
        simBuilder.RegisterSystem<game::scene::systems::debug::DebugAxisInputSystem>();
        simBuilder.RegisterSystem<game::scene::systems::cameras::CameraControllerSystem>();
    }

    // Rendering
    {
        const auto [width, height] = app_host::Application::Get().GetAppDimensions();
        m_Rendering.m_GBuffer.Initialise(width, height);
        setViewRect(constants::render_views::c_geometry, 0, 0, bgfx::BackbufferRatio::Equal);
        setViewFrameBuffer(constants::render_views::c_geometry, m_Rendering.m_GBuffer.GetHandle());

        render::addToFrameGraph("BufferPrep", [this]
        {
            const auto [newWidth, newHeight] = app_host::Application::Get().GetAppDimensions();
            m_Rendering.m_GBuffer.EnsureSize(newWidth, newHeight);
        });
        frameBuilder.RegisterSystem<game::scene::systems::cameras::CameraViewProjectionUpdateSystem>(constants::render_views::c_geometry);
        frameBuilder.RegisterSystem<game::scene::systems::debug::DebugAxisRenderSystem>(constants::render_views::c_geometry);
        frameBuilder.RegisterSystem<game::scene::systems::rendering::PostProcessSystem>(constants::render_views::c_postProcess, m_Rendering.m_GBuffer);
    }
}

void atlas::scene_editor::SceneEditorState::ClearScene()
{
    scene::EcsManager& ecs = GetEcsManager();
    ecs.Clear();

    addCameras(ecs);
    addLights(ecs);
    addDebugRenderingComponents(ecs);
}

atlas::scene::EntityId atlas::scene_editor::SceneEditorState::CreateEntity()
{
    return GetEcsManager().AddEntity();
}

void atlas::scene_editor::SceneEditorState::DeleteEntity(const scene::EntityId id)
{
    GetEcsManager().RemoveEntity(id);
}
