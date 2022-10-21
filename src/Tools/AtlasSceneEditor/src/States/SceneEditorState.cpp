#include "AtlasSceneEditorPCH.h"
#include "SceneEditorState.h"

#include "Constants.h"
#include "AtlasAppHost/Application.h"
#include "AtlasGame/Components/DebugAxisComponent.h"
#include "AtlasGame/Components/DirectionalLightComponent.h"
#include "AtlasGame/Components/LookAtCameraComponent.h"
#include "AtlasGame/Scene/Systems/TransformUpdateSystem.h"
#include "AtlasGame/Scene/Systems/Cameras/CameraControllerSystem.h"
#include "AtlasGame/Scene/Systems/Cameras/CameraViewProjectionUpdateSystem.h"
#include "AtlasGame/Scene/Systems/Debug/DebugAxisInputSystem.h"
#include "AtlasGame/Scene/Systems/Rendering/LightingSystem.h"
#include "AtlasGame/Scene/Systems/Rendering/ModelRenderSystem.h"
#include "AtlasGame/Scene/Systems/Rendering/PostProcessSystem.h"
#include "AtlasGame/Scene/Systems/Rendering/ShadowMappingSystem.h"
#include "AtlasGame/Scene/Systems/Interactivity/PickingSystem.h"
#include "AtlasGame/Scene/Systems/Interactivity/SelectionSystem.h"
#include "AtlasGame/Scene/Systems/Rendering/SelectionRenderingSystem.h"

namespace
{
    void addCameras(atlas::scene::EcsManager& ecs)
    {
        const auto cameraEntity = ecs.AddEntity();
        auto& camera2 = ecs.AddComponent<atlas::game::components::cameras::LookAtCameraComponent>(cameraEntity);
        camera2.m_IsRenderActive = true;
        camera2.m_IsControlActive = true;
        camera2.m_Distance = 5.0f;
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
        simBuilder.RegisterSystem<game::scene::systems::TransformUpdateSystem>();
    }

    // Rendering
    game::scene::systems::interactivity::PickingSystem* pickingSystem;
    {
        bgfx::setViewName(constants::render_views::c_shadowPass, "Shadow");
        bgfx::setViewName(constants::render_views::c_geometry, "Geometry");
        bgfx::setViewName(constants::render_views::c_debugGeometry, "DebugGeometry");
        bgfx::setViewName(constants::render_views::c_postProcess, "PostProcess");
        bgfx::setViewName(constants::render_views::c_ui, "UI");
        bgfx::setViewName(constants::render_views::c_debugui, "DebugUI");
        bgfx::setViewName(constants::render_views::c_debugVisualizerCopy, "DebugVisualizerCopy");

        const auto [width, height] = app_host::Application::Get().GetAppDimensions();
        m_Rendering.m_GBuffer.Initialise(width, height);
        setViewRect(constants::render_views::c_geometry, 0, 0, bgfx::BackbufferRatio::Equal);
        setViewRect(constants::render_views::c_debugGeometry, 0, 0, bgfx::BackbufferRatio::Equal);
        setViewFrameBuffer(constants::render_views::c_geometry, m_Rendering.m_GBuffer.GetHandle());
        setViewFrameBuffer(constants::render_views::c_debugGeometry, m_Rendering.m_GBuffer.GetHandle());

        render::addToFrameGraph("BufferPrep", [this]
        {
            const auto [newWidth, newHeight] = app_host::Application::Get().GetAppDimensions();
            m_Rendering.m_GBuffer.EnsureSize(newWidth, newHeight);
        });
        frameBuilder.RegisterSystem<game::scene::systems::cameras::CameraViewProjectionUpdateSystem>(
            std::vector {
                constants::render_views::c_geometry,
                constants::render_views::c_debugGeometry,
                constants::render_views::c_pickingViews[0],
                constants::render_views::c_selectionViews[0]
            });
        frameBuilder.RegisterSystem<game::scene::systems::rendering::ShadowMappingSystem>(constants::render_views::c_shadowPass, constants::render_masks::c_shadowCaster);
        frameBuilder.RegisterSystem<game::scene::systems::rendering::LightingSystem>();
        frameBuilder.RegisterSystem<game::scene::systems::rendering::ModelRenderSystem>(
            constants::render_views::c_geometry,
            std::vector<game::scene::systems::rendering::ModelRenderSystem::Pass> {
                {
                    constants::render_views::c_geometry,
                    constants::render_masks::c_generalGeometry,
                    BGFX_STATE_DEFAULT,
                    nullptr
                }
            });
        frameBuilder.RegisterSystem<game::scene::systems::rendering::SelectionRenderingSystem>(
            constants::render_views::c_selectionViews,
            m_Rendering.m_GBuffer.GetHandle());

        frameBuilder.RegisterSystem<game::scene::systems::debug::DebugAxisRenderSystem>(constants::render_views::c_geometry);

        const auto postProcess = frameBuilder.RegisterSystem<game::scene::systems::rendering::PostProcessSystem>(constants::render_views::c_postProcess, m_Rendering.m_GBuffer.GetHandle());
        pickingSystem = frameBuilder.RegisterSystem<game::scene::systems::interactivity::PickingSystem>(
            constants::render_views::c_pickingViews,
            constants::render_masks::c_pickable);

        frameBuilder.RegisterLambda("DebugRenderingDisplay",
            []
            {
                bgfx::setViewClear(constants::render_views::c_debugVisualizerCopy, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFFFFFFFF);
                setViewMode(constants::render_views::c_debugVisualizerCopy, bgfx::ViewMode::Sequential);
                setViewRect(constants::render_views::c_debugVisualizerCopy, 0, 0, bgfx::BackbufferRatio::Half);
            },
            [this, postProcess, pickingSystem]
            {
                switch(m_Rendering.m_DisplayState)
                {
                case DisplayState::DisplayNormal: /* Do Nothing */ break;
                case DisplayState::DisplayPickingBuffer:
                    postProcess->PerformCopy(
                        constants::render_views::c_debugVisualizerCopy,
                        getTexture(pickingSystem->GetPickingFrameBuffer()),
                        BGFX_INVALID_HANDLE);
                    break;
                }
            });
    }

    // Sim Extended
    {
        simBuilder.RegisterSystem<game::scene::systems::interactivity::SelectionSystem>(*pickingSystem);
    }
}

void atlas::scene_editor::SceneEditorState::ClearScene()
{
    scene::EcsManager& ecs = GetEcsManager();
    ecs.Clear();

    addCameras(ecs);
    addLights(ecs);
    //addDebugRenderingComponents(ecs);
}

atlas::scene::EntityId atlas::scene_editor::SceneEditorState::CreateEntity()
{
    return GetEcsManager().AddEntity();
}

void atlas::scene_editor::SceneEditorState::DeleteEntity(const scene::EntityId id)
{
    GetEcsManager().RemoveEntity(id);
}
