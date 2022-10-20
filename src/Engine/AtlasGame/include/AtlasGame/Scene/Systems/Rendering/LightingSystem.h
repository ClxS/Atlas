#pragma once
#include <bgfx/bgfx.h>

#include "AtlasRender/BgfxHandle.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace atlas::game::scene::systems::rendering
{
    constexpr uint8_t c_MaxDirectionalLights = 1;

    class LightingSystem final : public atlas::scene::SystemBase
    {
    public:
        [[nodiscard]] std::string_view GetName() const override { return "LightingSystem"; }

        void Initialise(atlas::scene::EcsManager&) override;
        void Render(atlas::scene::EcsManager& ecs) override;

    private:
        struct
        {
            std::array<render::BgfxHandle<bgfx::UniformHandle>, c_MaxDirectionalLights> m_LightDirections;
            std::array<render::BgfxHandle<bgfx::UniformHandle>, c_MaxDirectionalLights> m_LightColours;
            render::BgfxHandle<bgfx::UniformHandle> m_AmbientColour{BGFX_INVALID_HANDLE};
        } m_Uniforms{};
    };
}
