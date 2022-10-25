#include "AtlasInputPCH.h"
#include "UserInputManager.h"

#include "Keyboard.h"
#include "KeyboardControlScheme.h"
#include "Mouse.h"
#include "MouseControlScheme.h"
#include "SDL_events.h"
#include "SDL_keyboard.h"
#include "SDL_mouse.h"
#include "UserDevice_MouseKeyboard.h"

namespace
{
    bool g_wheelScrolled = false;
    float g_wheelX = 0.0f;
    float g_wheelY = 0.0f;
}

namespace atlas::input
{
    void clearMouseWheel_private()
    {
        g_wheelScrolled = false;
    }

    void setMouseWheel_private(const float x, const float y)
    {
        g_wheelScrolled = true;
        g_wheelX = x;
        g_wheelY = y;
    }
}

bool atlas::input::UserInputManager::Initialise()
{
    m_UserCount = 1;
    m_Users[0] = CreateMouseKeyboardUser();

    return true;
}

void atlas::input::UserInputManager::Tick() const
{
    UpdateKeyboards();
    UpdateMice();
}

atlas::input::UserInputManager::UserEntry atlas::input::UserInputManager::CreateMouseKeyboardUser() const
{
    return
    {
        UserEntry::Type::MouseKeyboard,
        std::make_unique<profiles::UserDevice_MouseKeyboard>
        (
            std::make_unique<devices::Mouse>(),
            std::make_unique<devices::Keyboard>(),
            m_ControlSchemes.m_Mouse.empty() ? nullptr : m_ControlSchemes.m_Mouse[0].get(),
            m_ControlSchemes.m_Keyboard.empty() ? nullptr : m_ControlSchemes.m_Keyboard[0].get()
        )
    };
}

void atlas::input::UserInputManager::UpdateKeyboards() const
{
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
    for(auto& profile : m_Users)
    {
        if (profile.m_Type != UserEntry::Type::MouseKeyboard || !profile.m_Profile)
        {
            continue;
        }

        const auto mouseKeyboardProfile = static_cast<profiles::UserDevice_MouseKeyboard*>(profile.m_Profile.get());
        if (!mouseKeyboardProfile->m_Keyboard)
        {
            continue;
        }

        const auto keyboard = mouseKeyboardProfile->m_Keyboard.get();
        for(int i = 0; i < SDL_NUM_SCANCODES; i++)
        {
            keyboard->m_Keys[i] = keyboardState[i] == SDL_PRESSED;
        }
    }
}

void atlas::input::UserInputManager::UpdateMice() const
{
    static int previousMouseX = 0;
    static int previousMouseY = 0;

    int mouseX, mouseY;
    const uint32_t buttons = SDL_GetMouseState(&mouseX, &mouseY);

    const int32_t mouseDeltaX = mouseX - previousMouseX;
    const int32_t mouseDeltaY = mouseY - previousMouseY;

    float wheelY = 0.0f;
    if (g_wheelScrolled)
    {
        wheelY = g_wheelY;
    }

    for(auto& profile : m_Users)
    {
        if (profile.m_Type != UserEntry::Type::MouseKeyboard || !profile.m_Profile)
        {
            continue;
        }

        const auto mouseKeyboardProfile = static_cast<profiles::UserDevice_MouseKeyboard*>(profile.m_Profile.get());
        if (mouseKeyboardProfile->m_Mouse)
        {
            mouseKeyboardProfile->m_Mouse->m_CurrentCoord = { mouseX, mouseY };
            mouseKeyboardProfile->m_Mouse->m_DeltaCoord = { mouseDeltaX, mouseDeltaY };
            mouseKeyboardProfile->m_Mouse->m_ScrollDelta = static_cast<int32_t>(wheelY);

            mouseKeyboardProfile->m_Mouse->m_ButtonState[static_cast<int>(devices::MouseButton::Left)] = buttons & SDL_BUTTON_LMASK;
            mouseKeyboardProfile->m_Mouse->m_ButtonState[static_cast<int>(devices::MouseButton::Right)] = buttons & SDL_BUTTON_RMASK;
            mouseKeyboardProfile->m_Mouse->m_ButtonState[static_cast<int>(devices::MouseButton::Middle)] = buttons & SDL_BUTTON_MMASK;
            mouseKeyboardProfile->m_Mouse->m_ButtonState[static_cast<int>(devices::MouseButton::X1)] = buttons & SDL_BUTTON_X1MASK;
            mouseKeyboardProfile->m_Mouse->m_ButtonState[static_cast<int>(devices::MouseButton::X2)] = buttons & SDL_BUTTON_X2MASK;
        }
    }

    previousMouseX = mouseX;
    previousMouseY = mouseY;
}

void atlas::input::UserInputManager::AddControlScheme(std::unique_ptr<control_schemes::KeyboardControlScheme>&& keyboardControlScheme)
{
    for(int i = 0; i < m_UserCount; i++)
    {
        if (m_Users[i].m_Type == UserEntry::Type::MouseKeyboard && m_Users[i].m_Profile)
        {
            auto* deviceMouseKeyboard = static_cast<profiles::UserDevice_MouseKeyboard*>(m_Users[i].m_Profile.get());
            if (!deviceMouseKeyboard->GetKeyboardControlScheme())
            {
                deviceMouseKeyboard->SetKeyboardControlScheme(keyboardControlScheme.get());
            }
        }
    }

    m_ControlSchemes.m_Keyboard.emplace_back(std::move(keyboardControlScheme));
}

void atlas::input::UserInputManager::AddControlScheme(std::unique_ptr<control_schemes::MouseControlScheme>&& mouseControlScheme)
{
    for(int i = 0; i < m_UserCount; i++)
    {
        if (m_Users[i].m_Type == UserEntry::Type::MouseKeyboard && m_Users[i].m_Profile)
        {
            auto* deviceMouseKeyboard = static_cast<profiles::UserDevice_MouseKeyboard*>(m_Users[i].m_Profile.get());
            if (!deviceMouseKeyboard->GetMouseControlScheme())
            {
                deviceMouseKeyboard->SetMouseControlScheme(mouseControlScheme.get());
            }
        }
    }

    m_ControlSchemes.m_Mouse.emplace_back(std::move(mouseControlScheme));
}

const atlas::input::profiles::UserDevice* atlas::input::UserInputManager::GetUser(const int32_t slot) const
{
    if (slot >= m_UserCount || slot < 0)
    {
        return nullptr;
    }

    return m_Users[slot].m_Profile.get();
}
