#pragma once
#include "RmlUi/Core/SystemInterface.h"

namespace atlas::ui::rml
{
    class RmlSystemInterface final : public Rml::SystemInterface
    {
    public:
        double GetElapsedTime() override;
    };
}
