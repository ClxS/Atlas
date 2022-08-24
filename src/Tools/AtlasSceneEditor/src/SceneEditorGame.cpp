﻿#include "AtlasSceneEditorPCH.h"

#include "SceneEditorState.h"
#include "AtlasGame/GameHost.h"
#include "Utility/Constants.h"

namespace
{
    void registerComponents()
    {
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
