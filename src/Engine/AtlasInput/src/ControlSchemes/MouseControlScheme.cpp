#include "AtlasInputPCH.h"
#include "MouseControlScheme.h"

void atlas::input::control_schemes::MouseControlScheme::SetMapping(const core::StringHashView view,
    const devices::MouseAxis axis)
{
    m_AxisMappings[core::StringHash{view}] = axis;
}

void atlas::input::control_schemes::MouseControlScheme::SetMapping(const core::StringHashView view,
    const devices::MouseButton button)
{
    m_ButtonMappings[core::StringHash{view}] = button;
}

std::optional<atlas::input::devices::MouseAxis> atlas::input::control_schemes::MouseControlScheme::GetAxis(
    const core::StringHashView axis)
{
    const auto iter = m_AxisMappings.find(axis);
    if (iter == m_AxisMappings.end())
    {
        return {};
    }

    return iter->second;
}

std::optional<atlas::input::devices::MouseButton> atlas::input::control_schemes::MouseControlScheme::GetButton(
    const core::StringHashView button)
{
    const auto iter = m_ButtonMappings.find(button);
    if (iter == m_ButtonMappings.end())
    {
        return {};
    }

    return iter->second;
}
