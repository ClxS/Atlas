#pragma once
#include <optional>
#include <unordered_map>

#include "ControlScheme.h"
#include "AtlasCore/StringHash.h"
#include "AtlasInput/Devices/MouseAxis.h"
#include "AtlasInput/Devices/MouseButton.h"

namespace atlas::input::control_schemes
{
    class MouseControlScheme : public ControlScheme
    {
    public:
        explicit MouseControlScheme(std::string name)
            : ControlScheme{std::move(name)}
        {
        }

        void SetMapping(core::StringHashView view, devices::MouseAxis axis);

        void SetMapping(core::StringHashView view, devices::MouseButton button);

        [[nodiscard]] std::optional<devices::MouseAxis> GetAxis(core::StringHashView axis);
        [[nodiscard]] std::optional<devices::MouseButton> GetButton(core::StringHashView button);

    private:
        std::unordered_map<core::StringHash, devices::MouseAxis, core::comparators::StringHash, core::comparators::StringEqual> m_AxisMappings;
        std::unordered_map<core::StringHash, devices::MouseButton, core::comparators::StringHash, core::comparators::StringEqual> m_ButtonMappings;
    };
}
