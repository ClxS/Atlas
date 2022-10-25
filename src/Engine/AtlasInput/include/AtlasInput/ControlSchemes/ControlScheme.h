#pragma once
#include "AtlasCore/StringHash.h"

namespace atlas::input::control_schemes
{
    class ControlScheme
    {
    public:
        explicit ControlScheme(std::string name)
            : m_Name{std::move(name)}
        {
        }

        [[nodiscard]] std::string_view GetName() const { return m_Name; }

    private:
        std::string m_Name;
    };
}
