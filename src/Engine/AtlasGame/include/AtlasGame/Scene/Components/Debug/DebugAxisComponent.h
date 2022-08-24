#pragma once
#include "AtlasCore/Colour.h"

namespace atlas::game::scene::components::debug
{
    struct DebugAxisComponent
    {
        core::Colour32 m_XAxisColour = 0xff0000ff_argb;
        core::Colour32 m_YAxisColour = 0xff00ff00_argb;
        core::Colour32 m_ZAxisColour = 0xffff0000_argb;

        int m_Length = 15;
        int m_Thickness = 1;
    };
}
