#pragma once
#include <optional>
#include <unordered_map>

#include "ControlScheme.h"
#include "AtlasCore/StringHash.h"
#include "AtlasInput/Devices/KeyboardScanCodes.h"

namespace atlas::input::control_schemes
{
    class KeyboardControlScheme : public ControlScheme
    {
    public:
        explicit KeyboardControlScheme(std::string name)
            : ControlScheme{std::move(name)}
        {
        }

        void SetMapping(const core::StringHashView view, const devices::keys::Enum value)
        {
            m_Mapping[core::StringHash{view}] = value;
        }

        [[nodiscard]] std::optional<devices::keys::Enum> GetKey(core::StringHashView key);

    private:
        std::unordered_map<core::StringHash, devices::keys::Enum, core::comparators::StringHash, core::comparators::StringEqual> m_Mapping;
    };
}
