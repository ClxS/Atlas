#pragma once

#include <future>

#include "PickingSystem.h"
#include "AtlasScene/ECS/Entity.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace atlas::game::scene::systems::interactivity
{
    class SelectionSystem final : public atlas::scene::SystemBase
    {
    public:
        explicit SelectionSystem(PickingSystem& pickingSystem)
            : m_PickingSystem{pickingSystem}
        {
        }

        [[nodiscard]] std::string_view GetName() const override { return "SelectionSystem"; }

        void Initialise(atlas::scene::EcsManager& ecs) override;
        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        struct PickRequest
        {
            std::future<atlas::scene::EntityId> m_Request;
            bool m_IsComplete{false};
        };

        static void ConsumePick(atlas::scene::EcsManager& ecs, PickRequest& value);

        PickingSystem& m_PickingSystem;
        bool m_IsMouseDown{false};
        uint32_t m_MouseDownTime{0};

        std::vector<PickRequest> m_PickRequests;
    };
}
