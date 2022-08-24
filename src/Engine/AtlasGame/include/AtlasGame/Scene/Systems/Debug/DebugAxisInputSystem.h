#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace atlas::game::scene::systems::debug
{
    class DebugAxisInputSystem final : public atlas::scene::SystemBase
    {
    public:
        std::string_view GetName() const override { return "DebugAxisInputSystem"; }

        void Initialise(atlas::scene::EcsManager&) override;
        void Update(atlas::scene::EcsManager& ecs) override;
    };
}
