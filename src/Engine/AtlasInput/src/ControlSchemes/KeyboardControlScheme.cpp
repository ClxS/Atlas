#include "AtlasInputPCH.h"
#include "KeyboardControlScheme.h"

std::optional<atlas::input::devices::keys::Enum> atlas::input::control_schemes::KeyboardControlScheme::GetKey(
    const core::StringHashView key)
{
    const auto iter = m_Mapping.find(key);
    if (iter == m_Mapping.end())
    {
        return {};
    }

    return iter->second;
}
