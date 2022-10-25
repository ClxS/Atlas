#include "AtlasInputPCH.h"
#include "UserDevice_MouseKeyboard.h"
#include "MouseControlScheme.h"
#include "KeyboardControlScheme.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "PointerCoord.h"

bool atlas::input::profiles::UserDevice_MouseKeyboard::IsButtonDown(const core::StringHashView button) const
{
    if (m_KeyboardControlScheme && m_Keyboard)
    {
        const auto key = m_KeyboardControlScheme->GetKey(button);
        if (key.has_value() && m_Keyboard->IsKeyDown(key.value()))
        {
            return true;
        }
    }

    if (m_MouseControlScheme && m_Mouse)
    {
        const auto key = m_MouseControlScheme->GetButton(button);
        if (key.has_value() && m_Mouse->IsButtonDown(key.value()))
        {
            return true;
        }
    }

    return false;
}

bool atlas::input::profiles::UserDevice_MouseKeyboard::IsButtonMapped(const core::StringHashView button) const
{
    if (m_MouseControlScheme && m_MouseControlScheme->GetButton(button).has_value())
    {
        return true;
    }

    if (m_KeyboardControlScheme && m_KeyboardControlScheme->GetKey(button).has_value())
    {
        return true;
    }

    return false;
}

int32_t atlas::input::profiles::UserDevice_MouseKeyboard::GetAxisValue(core::StringHashView axis) const
{
    if (!m_MouseControlScheme || !m_Mouse)
    {
        return 0;
    }

    const auto axisMap = m_MouseControlScheme->GetAxis(axis);
    if (!axisMap.has_value())
    {
        return 0;
    }

    return m_Mouse->GetAxis(axisMap.value());
}

int32_t atlas::input::profiles::UserDevice_MouseKeyboard::GetAxisAbsoluteValue(core::StringHashView axis) const
{
    if (!m_MouseControlScheme || !m_Mouse)
    {
        return 0;
    }

    const auto axisMap = m_MouseControlScheme->GetAxis(axis);
    if (!axisMap.has_value())
    {
        return 0;
    }

    return m_Mouse->GetAxisAbsolute(axisMap.value());
}

uint8_t atlas::input::profiles::UserDevice_MouseKeyboard::GetPointerCount() const
{
    return m_Mouse ? 1 : 0;
}

atlas::input::PointerCoord atlas::input::profiles::UserDevice_MouseKeyboard::GetPointerPosition(int32_t pointerIndex) const
{
    return m_Mouse->GetPosition();
}

atlas::input::PointerCoord atlas::input::profiles::UserDevice_MouseKeyboard::GetPointerDeltaPosition(
    int32_t pointerIndex) const
{
    return {};
}
