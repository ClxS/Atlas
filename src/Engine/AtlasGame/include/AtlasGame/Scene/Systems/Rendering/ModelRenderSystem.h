#pragma once
#include <bgfx/bgfx.h>
#include "AtlasScene/ECS/Systems/SystemBase.h"

#include "AtlasResource/AssetPtr.h"
#include "AtlasScene/ECS/Entity.h"

namespace atlas::render
{
    class ShaderProgram;
}

namespace atlas::game::scene::systems::rendering
{
    class ModelRenderSystem : public atlas::scene::SystemBase
    {
    public:
        struct Pass
        {
            bgfx::ViewId m_ViewId;
            uint8_t m_RenderMask;
            uint64_t m_State;
            resource::AssetPtr<render::ShaderProgram> m_bOverrideProgram;
        };

        ModelRenderSystem(
            bgfx::ViewId view,
            std::vector<Pass>&& passes,
            bool useInstancedRendering = true,
            std::function<void(atlas::scene::EntityId)> preRenderCallback = nullptr);

        [[nodiscard]] std::string_view GetName() const override { return "ModelRenderSystem"; }

        void Render(atlas::scene::EcsManager& ecs) override;

    private:
        bgfx::ViewId m_View;
        uint32_t m_RenderMask;

        std::vector<Pass> m_Passes;
        bool m_UseInstancedRendering;

        std::function<void(atlas::scene::EntityId)> m_PreRenderCallback;
    };
}
