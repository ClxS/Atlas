#include "AtlasSceneEditorPCH.h"
#include "SceneEditor.h"

#include "InstanceInteractionService.h"
#include "Registry.h"
#include "SceneEditingService.h"
#include "SceneEditorState.h"
#include "AtlasGame/GameHost.h"
#include "AtlasGame/Components/ModelComponent.h"
#include "AtlasInput/ControlSchemes/KeyboardControlScheme.h"
#include "Utility/Constants.h"
#include "AtlasGame/Controls/InputId.h"

void atlas::scene_editor::SceneEditorGame::OnStartup()
{
    using namespace resource;
    using namespace scene;

    RegisterComponents();
    RegisterTypeHandlers();
    RegisterAssetBundles();
    LoadDataAssets();
    ConfigureBgfx();
    RegisterControlSchemes();

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

void atlas::scene_editor::SceneEditorGame::RegisterControlSchemes()
{
    using namespace input::control_schemes;
    using namespace input::devices;
    using namespace game::controls;

    auto keyboard = std::make_unique<KeyboardControlScheme>("Keyboard_Default_1");
    keyboard->SetMapping(input_id::c_buttonForward, keys::W);
    keyboard->SetMapping(input_id::c_buttonLeft, keys::A);
    keyboard->SetMapping(input_id::c_buttonRight, keys::D);
    keyboard->SetMapping(input_id::c_buttonBack, keys::S);
    keyboard->SetMapping(input_id::c_buttonFocus, keys::F);
    keyboard->SetMapping(input_id::c_buttonVerticalPanModifier, keys::LShift);
    keyboard->SetMapping(input_id::c_buttonExtendSelection, keys::LCtrl);

    auto mouse = std::make_unique<MouseControlScheme>("Mouse_Default_1");
    mouse->SetMapping(input_id::c_axisZoom, MouseAxis::Wheel);
    mouse->SetMapping(input_id::c_axisPitch, MouseAxis::Y);
    mouse->SetMapping(input_id::c_axisYaw, MouseAxis::X);
    mouse->SetMapping(input_id::c_buttonPanModifier, MouseButton::Middle);
    mouse->SetMapping(input_id::c_buttonLeftTouch, MouseButton::Left);

    input::UserInputManager::Get().AddControlScheme(std::move(keyboard));
    input::UserInputManager::Get().AddControlScheme(std::move(mouse));
}
