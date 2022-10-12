#include "AtlasSceneEditorPCH.h"

#include "InstanceInteractionService.h"
#include "Registry.h"
#include "SceneEditingService.h"
#include "SceneEditorState.h"
#include "AtlasGame/GameHost.h"
#include "Utility/Constants.h"

namespace
{
    void registerComponents()
    {
        atlas::scene_editor::buildComponentRegistry();
    }

    void registerAssetBundles()
    {
    }

    void registerTypeHandlers()
    {
    }

    void loadDataAssets()
    {
    }

    void setBgfxSettings()
    {
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF, 1.0f, 0);
        bgfx::setViewClear(atlas::scene_editor::constants::render_views::c_geometry, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF, 1.0f, 0);
    }

    class SceneEditorGame final : public atlas::game::GameImplementation
    {
    public:
        void OnStartup() override
        {
            using namespace atlas::resource;
            using namespace atlas::scene;

            registerComponents();
            registerTypeHandlers();
            registerAssetBundles();
            loadDataAssets();
            setBgfxSettings();

            m_EditorState = &m_SceneManager.TransitionTo<atlas::scene_editor::SceneEditorState>();
        }

        void OnInitialised() override
        {
            grpc::ServerContext* context{nullptr};
            Unit unit{};
            Result result{};
            Entity entity{};

            m_EditorRpc->CreateNewScene(context, &unit, &result);

            m_EditorRpc->CreateEntity(context, &unit, &entity);
#error continue
        }

        void RegisterRpc(atlas::rpc::RpcServer& server) override
        {
            server.RegisterService<atlas::scene_editor::rpc::InstanceInteractionServiceImpl>();
            m_EditorRpc = server.RegisterService<atlas::scene_editor::rpc::SceneEditingServiceImpl>(m_EditorState);
        }

    private:
        atlas::scene_editor::SceneEditorState* m_EditorState{nullptr};

        atlas::scene_editor::rpc::SceneEditingServiceImpl* m_EditorRpc{nullptr};
    };
}

int gameMain(int argc, char* argv[])
{
    atlas::game::GameHost<SceneEditorGame> game{
            {
                "Scene Editor",
                atlas::scene_editor::constants::render_views::c_ui,
                atlas::scene_editor::constants::render_views::c_debugui,
                60
            }};
    return game.Run();
}


