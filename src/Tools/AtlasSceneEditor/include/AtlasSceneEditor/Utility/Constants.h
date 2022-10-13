#pragma once
#include "bgfx/bgfx.h"

namespace atlas::scene_editor::constants
{
    namespace render_views
    {
        constexpr bgfx::ViewId c_geometry = 0;
        constexpr bgfx::ViewId c_postProcess = 1;
        constexpr bgfx::ViewId c_ui = 90;
        constexpr bgfx::ViewId c_debugui = 91;
    }

    namespace render_masks
    {
    }
}
