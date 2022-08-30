#include "AtlasSceneEditorPCH.h"

#include "SceneEditorState.h"
#include "TestService.h"
#include "AtlasGame/GameHost.h"
#include "AtlasGame/Scene/Components/PositionComponent.h"
#include "AtlasGame/Scene/Components/Cameras/FreeCameraComponent.h"
#include "AtlasGame/Scene/Components/Cameras/LookAtCameraComponent.h"
#include "AtlasGame/Scene/Components/Cameras/SphericalLookAtCameraComponent.h"
#include "AtlasGame/Scene/Components/Lighting/DirectionalLightComponent.h"
#include "Utility/Constants.h"

namespace
{
    void registerComponents()
    {
        using namespace atlas::resource;
        using namespace atlas::scene;
        using namespace atlas::game::scene::systems::cameras;
        ComponentRegistry::RegisterComponent<atlas::game::scene::components::cameras::LookAtCameraComponent>();
        ComponentRegistry::RegisterComponent<atlas::game::scene::components::cameras::SphericalLookAtCameraComponent>();
        ComponentRegistry::RegisterComponent<atlas::game::scene::components::cameras::SphericalLookAtCameraComponent_Private>();
        ComponentRegistry::RegisterComponent<atlas::game::scene::components::cameras::FreeCameraComponent>();
        ComponentRegistry::RegisterComponent<atlas::game::scene::components::PositionComponent>();
        ComponentRegistry::RegisterComponent<atlas::game::scene::components::cameras::DirectionalLightComponent>();
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

            m_SceneManager.TransitionTo<atlas::scene_editor::SceneEditorState>();
        }

        void RegisterRpc(atlas::rpc::RpcServer& server) override
        {
            server.RegisterService<atlas::scene_editor::rpc::TestServiceImpl>();
        }
    };
}

int gameMain(int argc, char* argv[])
{
    atlas::game::GameHost<SceneEditorGame> game{
            {
                "Fayre",
                atlas::scene_editor::constants::render_views::c_ui,
                60
            }};
    return game.Run();
}
