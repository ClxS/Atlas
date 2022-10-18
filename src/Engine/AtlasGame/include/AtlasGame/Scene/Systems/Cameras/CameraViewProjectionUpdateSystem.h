#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "bgfx/bgfx.h"

namespace atlas::game::scene::systems::cameras
{
    class CameraViewProjectionUpdateSystem final : public atlas::scene::SystemBase
    {
    public:
        [[nodiscard]] std::string_view GetName() const override { return "CameraViewProjectionUpdateSystem"; }

        explicit CameraViewProjectionUpdateSystem(std::vector<bgfx::ViewId>&& viewIds) : m_ViewIds{viewIds} {}
        void Initialise(atlas::scene::EcsManager&) override;
        void Render(atlas::scene::EcsManager& ecs) override;

        void SetDebugRenderingEnabled(bool bEnabled);
    private:
        std::vector<bgfx::ViewId> m_ViewIds;
        bool m_bDebugRenderingEnabled{true};
    };
}
