#pragma once

#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace atlas::game::scene::systems::cameras
{
    class CameraControllerSystem final : public atlas::scene::SystemBase
    {
    public:
        std::string_view GetName() const override { return "CameraControllerSystem"; }

        void Update(atlas::scene::EcsManager&) override;
    };
}
