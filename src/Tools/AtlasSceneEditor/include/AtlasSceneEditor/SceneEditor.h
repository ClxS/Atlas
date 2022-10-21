#pragma once

#include "AtlasGame/GameHost.h"
#include "Utility/Constants.h"

namespace atlas::scene_editor
{
    namespace rpc
    {
        class SceneEditingServiceImpl;
    }

    class SceneEditorGame : public game::GameImplementation
    {
    public:
        void OnStartup() override;
        void RegisterRpc(atlas::rpc::RpcServer& server) override;

    protected:
        virtual void RegisterComponents();
        virtual void RegisterAssetBundles();
        virtual void RegisterTypeHandlers();
        virtual void LoadDataAssets();
        virtual void ConfigureBgfx();

        class SceneEditorState* m_EditorState{nullptr};
        rpc::SceneEditingServiceImpl* m_EditorRpc{nullptr};
    };

    template<typename T>
    int runSceneEditor([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
    {
        game::GameHost<T> game{
                {
                    "Scene Editor",
                    constants::render_views::c_ui,
                    constants::render_views::c_debugui,
                    constants::render_views::c_debugGeometry,
                    60
                }};
        return game.Run();
    }
}
