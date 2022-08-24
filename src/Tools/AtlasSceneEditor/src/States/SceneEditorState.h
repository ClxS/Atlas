#pragma once
#include "AtlasGame/Scene/Systems/Debug/DebugAxisRenderSystem.h"
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
    };
}
