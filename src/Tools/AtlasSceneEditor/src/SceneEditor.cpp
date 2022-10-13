#include "AtlasSceneEditorPCH.h"
#include "SceneEditor.h"

#include "InstanceInteractionService.h"
#include "Registry.h"
#include "SceneEditingService.h"
#include "SceneEditorState.h"
#include "AtlasGame/GameHost.h"
#include "AtlasGame/Components/ModelComponent.h"
#include "Utility/Constants.h"

void atlas::scene_editor::SceneEditorGame::OnStartup()
{
    using namespace resource;
    using namespace scene;

    RegisterComponents();
    RegisterTypeHandlers();
    RegisterAssetBundles();
    LoadDataAssets();
    ConfigureBgfx();

    m_EditorState = &m_SceneManager.TransitionTo<SceneEditorState>();
}

void atlas::scene_editor::SceneEditorGame::RegisterRpc(atlas::rpc::RpcServer& server)
{
    server.RegisterService<rpc::InstanceInteractionServiceImpl>();
    m_EditorRpc = server.RegisterService<rpc::SceneEditingServiceImpl>(m_EditorState);
}

void atlas::scene_editor::SceneEditorGame::RegisterComponents()
{
    buildComponentRegistry();
}

void atlas::scene_editor::SceneEditorGame::RegisterAssetBundles()
{
}

void atlas::scene_editor::SceneEditorGame::RegisterTypeHandlers()
{
}

void atlas::scene_editor::SceneEditorGame::LoadDataAssets()
{
}

void atlas::scene_editor::SceneEditorGame::ConfigureBgfx()
{
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF, 1.0f, 0);
    bgfx::setViewClear(constants::render_views::c_geometry, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF, 1.0f, 0);
}


