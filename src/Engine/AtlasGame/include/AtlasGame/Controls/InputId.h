#pragma once
#include "AtlasCore/StringHash.h"

namespace atlas::game::controls::input_id
{
    inline static const core::StringHash c_buttonForward {"Forward"};
    inline static const core::StringHash c_buttonBack {"Back"};
    inline static const core::StringHash c_buttonLeft {"Left"};
    inline static const core::StringHash c_buttonRight {"Right"};
    inline static const core::StringHash c_buttonFocus {"Focus"};
    inline static const core::StringHash c_buttonPanModifier {"PanModifier"};
    inline static const core::StringHash c_buttonVerticalPanModifier {"VerticalPanModifier"};
    inline static const core::StringHash c_buttonLeftTouch {"LeftTouch"};
    inline static const core::StringHash c_buttonExtendSelection {"ExtendSelection"};

    inline static const core::StringHash c_axisZoom {"Zoom"};
    inline static const core::StringHash c_axisYaw {"Yaw"};
    inline static const core::StringHash c_axisPitch {"Pitch"};

}
