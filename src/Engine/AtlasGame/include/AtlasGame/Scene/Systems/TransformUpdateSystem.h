#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace atlas::game::scene::systems
{
    class TransformUpdateSystem final : public atlas::scene::SystemBase
    {
    public:
        [[nodiscard]] std::string_view GetName() const override { return "TransformUpdateSystem"; }

        void Update(atlas::scene::EcsManager& ecs) override;
    };
}
