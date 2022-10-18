#pragma once
#include "AtlasGame/Scene/Systems/Debug/DebugAxisRenderSystem.h"
#include "AtlasRender/Types/FrameBuffer.h"
#include "AtlasScene/Scene.h"

namespace atlas::ui
{
    class UIPage;
}

namespace atlas::game::scene::systems::cameras
{
    class CameraViewProjectionUpdateSystem;
}

namespace atlas::scene_editor
{
    class SceneEditorState final : public scene::EcsScene
    {
    public:
        void OnEntered(scene::SceneManager& sceneManager) override;
        void ConstructSystems(scene::SystemsBuilder& simBuilder, scene::SystemsBuilder& frameBuilder) override;

        void ClearScene();

        scene::EntityId CreateEntity();
        void DeleteEntity(scene::EntityId id);

    private:
        enum class DisplayState
        {
            DisplayNormal,
            DisplayPickingBuffer
        };

        struct
        {
            render::FrameBuffer m_GBuffer;
            DisplayState m_DisplayState{ DisplayState::DisplayNormal };
        } m_Rendering;
    };
}
