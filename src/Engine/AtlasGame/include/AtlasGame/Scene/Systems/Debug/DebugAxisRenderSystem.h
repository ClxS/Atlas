﻿#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "bgfx/bgfx.h"

namespace atlas::game::scene::systems::debug
{
    class DebugAxisRenderSystem final : public atlas::scene::SystemBase
    {
    public:
        explicit DebugAxisRenderSystem(bgfx::ViewId view);

        [[nodiscard]] std::string_view GetName() const override { return "DebugAxisRenderSystem"; }

        void Initialise(atlas::scene::EcsManager&) override;
        void Render(atlas::scene::EcsManager& ecs) override;

    private:
        bgfx::ViewId m_View;
    };
}
