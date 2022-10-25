#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace atlas::scene_editor
{
    class SceneInteractionSystem final : public scene::SystemBase
    {
    public:
        [[nodiscard]] std::string_view GetName() const override { return "SceneInteractionSystem"; }

        void Update(scene::EcsManager& ecs) override;
    };
}
