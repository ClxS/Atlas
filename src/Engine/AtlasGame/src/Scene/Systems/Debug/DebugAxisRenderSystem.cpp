#include "AtlasGamePCH.h"
#include "DebugAxisRenderSystem.h"

#include <numeric>

#include "DebugAxisComponent.h"
#include "SelectionComponent.h"
#include "TransformComponent.h"
#include "AtlasCore/Colour.h"
#include "AtlasRender/Debug/DebugDraw.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "ViewTransformCache.h"
#include "AtlasAppHost/Application.h"

#include <imgui.h>
#include "ImGuizmo.h"
#include "AtlasTrace/Logging.h"


atlas::game::scene::systems::debug::DebugAxisRenderSystem::DebugAxisRenderSystem(bgfx::ViewId view): m_View{view}
{
}

void atlas::game::scene::systems::debug::DebugAxisRenderSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    SystemBase::Initialise(ecsManager);
}

void atlas::game::scene::systems::debug::DebugAxisRenderSystem::Render(atlas::scene::EcsManager& ecs)
{
    const Eigen::Matrix4f identity = Eigen::Matrix4f::Identity();

    auto [width, height] = app_host::Application::Get().GetAppDimensions();
    ImGuizmo::SetRect(0, 0, static_cast<float>(width), static_cast<float>(height));

    render::debug::debug_draw::setColor(core::colours::c_white);
    render::debug::debug_draw::drawGrid({ 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 100);

    std::vector<atlas::scene::EntityId> manipulatedEntities{};
    std::vector<Eigen::Vector3f> selectedPositions{};
    for(auto [entity, transform, _] : ecs.IterateEntityComponents<
            components::TransformComponent,
            components::interaction::SelectionComponent>())
    {
        selectedPositions.push_back(transform.m_Position);
        manipulatedEntities.push_back(entity);
    }

    if (selectedPositions.empty())
    {
        return;
    }

    const Eigen::Vector3f centrePoint = std::accumulate(
        selectedPositions.begin(),
        selectedPositions.end(),
        Eigen::Vector3f{0, 0, 0}) / selectedPositions.size();

    auto& viewTransform = utility::ViewTransformCache::GetViewTransform(m_View);
    for (auto [_, debugAxis] : ecs.IterateEntityComponents<components::debug::DebugAxisComponent>())
    {
        Eigen::Matrix4f mtx;
        ImGuizmo::RecomposeMatrixFromComponents(centrePoint.data(), identity.data(), identity.data(), mtx.data());
        if (Manipulate(
                viewTransform.m_View.data()
                , viewTransform.m_Projection.data()
                , ImGuizmo::TRANSLATE
                , ImGuizmo::LOCAL
                , mtx.data()
                ))
        {
            Eigen::Vector3f tmp;
            Eigen::Vector3f newTransform;
            ImGuizmo::DecomposeMatrixToComponents(mtx.data(), newTransform.data(), tmp.data(), tmp.data());
            Eigen::Vector3f delta = newTransform - centrePoint;
            for (const auto entity : manipulatedEntities)
            {
                auto& transform = ecs.GetComponent<components::TransformComponent>(entity);
                transform.m_Position += delta;
            }
        }
        break;
    }
}
