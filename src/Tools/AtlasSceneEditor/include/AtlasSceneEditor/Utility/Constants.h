﻿#pragma once
#include "bgfx/bgfx.h"

namespace atlas::scene_editor::constants
{
    namespace render_views
    {
        constexpr bgfx::ViewId c_shadowPass = 0;
        constexpr bgfx::ViewId c_geometry = 1;
        constexpr bgfx::ViewId c_debugGeometry = 2;
        constexpr std::array<bgfx::ViewId, 4> c_selectionViews = { 3, 4, 5, 6 };
        constexpr bgfx::ViewId c_postProcess = 10;
        constexpr bgfx::ViewId c_ui = 90;
        constexpr bgfx::ViewId c_debugui = 91;
        constexpr std::array<bgfx::ViewId, 2> c_pickingViews = { 92, 93 };
        constexpr bgfx::ViewId c_debugVisualizerCopy = 128;
    }

    namespace render_masks
    {
        constexpr uint8_t c_generalGeometry         = 1 << 0;
        constexpr uint8_t c_surfaceClippedGeometry  = 1 << 1;
        constexpr uint8_t c_clipCasterGeometry      = 1 << 2;
        constexpr uint8_t c_shadowCaster            = 1 << 3;
        constexpr uint8_t c_pickable                = 1 << 4;
    }
}
