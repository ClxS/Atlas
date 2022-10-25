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
#include "LookAtCameraComponent.h"
#include "AtlasTrace/Logging.h"

namespace
{
    void drawGrid()
    {
        atlas::render::debug::debug_draw::setColor(atlas::core::colours::c_white);
        atlas::render::debug::debug_draw::drawGrid({ 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 100);
    }

    void handleGizmos(
        atlas::scene::EcsManager& ecs,
        const bgfx::ViewId viewId,
        const atlas::game::scene::systems::debug::DebugAxisRenderSystem::ManipulatorType type,
        const atlas::game::scene::systems::debug::DebugAxisRenderSystem::TransformSpace space
        )
    {
        using namespace atlas::game;
        using namespace scene::systems::debug;
        const Eigen::Matrix4f identity = Eigen::Matrix4f::Identity();

        std::vector<atlas::scene::EntityId> manipulatedEntities{};
        std::vector<components::TransformComponent*> selectedPositions{};
        for(auto [entity, transform, _] : ecs.IterateEntityComponents<
                components::TransformComponent,
                components::interaction::SelectionComponent>())
        {
            selectedPositions.push_back(&transform);
            manipulatedEntities.push_back(entity);
        }

        if (selectedPositions.empty())
        {
            return;
        }

        const Eigen::Vector3f centrePoint = std::accumulate(
            selectedPositions.begin(),
            selectedPositions.end(),
            Eigen::Vector3f{0, 0, 0},
            [](const Eigen::Vector3f& current, components::TransformComponent* component) { return current + component->m_Position; })
        / selectedPositions.size();

        Eigen::Quaternionf rotation { Eigen::Quaternionf::Identity() };
        Eigen::Vector3f oldScale { 1.0f, 1.0f, 1.0f};

        if (selectedPositions.size() == 1)
        {
            rotation = selectedPositions[0]->m_Orientation;
            oldScale = selectedPositions[0]->m_Scale;
        }

        ImGuizmo::OPERATION op = ImGuizmo::OPERATION::TRANSLATE;
        switch(type)
        {
        case DebugAxisRenderSystem::ManipulatorType::Translate:
            op = ImGuizmo::OPERATION::TRANSLATE;
            break;
        case DebugAxisRenderSystem::ManipulatorType::Rotation:
            op = ImGuizmo::OPERATION::ROTATE;
            break;
        case DebugAxisRenderSystem::ManipulatorType::Scale:
            op = ImGuizmo::OPERATION::SCALE;
            break;
        }

        ImGuizmo::MODE mode { ImGuizmo::MODE::WORLD };
        switch(space)
        {
        case DebugAxisRenderSystem::TransformSpace::Local:
            mode = ImGuizmo::MODE::LOCAL;
            break;
        case DebugAxisRenderSystem::TransformSpace::World:
            mode = ImGuizmo::MODE::WORLD;
            break;
        }

        auto& viewTransform = utility::ViewTransformCache::GetViewTransform(viewId);
        for (auto [_, debugAxis] : ecs.IterateEntityComponents<components::debug::DebugAxisComponent>())
        {
            Eigen::Matrix4f mtx = atlas::maths_helpers::composeModelMatrix(centrePoint, oldScale, rotation);

            Eigen::Matrix4f deltaMatrix {Eigen::Matrix4f::Identity()};
            if (Manipulate(
                    viewTransform.m_View.data()
                    , viewTransform.m_Projection.data()
                    , op
                    , mode
                    , mtx.data()
                    , deltaMatrix.data()
                    ))
            {
                Eigen::Vector3f tmp;
                Eigen::Vector3f newScale;
                ImGuizmo::DecomposeMatrixToComponents(mtx.data(), tmp.data(), tmp.data(), newScale.data());

                switch(type)
                {
                case DebugAxisRenderSystem::ManipulatorType::Translate:
                    {
                        const Eigen::Vector3f delta = deltaMatrix.block<3, 1>(0, 3);
                        for (const auto entity : manipulatedEntities)
                        {
                            auto& transform = ecs.GetComponent<components::TransformComponent>(entity);
                            transform.m_Position += delta;
                        }
                        break;
                    }
                case DebugAxisRenderSystem::ManipulatorType::Rotation:
                    {
                        Eigen::Quaternionf deltaQuat{deltaMatrix.topLeftCorner<3, 3>()};
                        for (const auto entity : manipulatedEntities)
                        {
                            auto& transform = ecs.GetComponent<components::TransformComponent>(entity);
                            transform.m_Orientation = deltaQuat * transform.m_Orientation;
                        }
                        break;
                    }
                case DebugAxisRenderSystem::ManipulatorType::Scale:
                    {
                        const Eigen::Vector3f scaleDelta = newScale - oldScale;
                        for (const auto entity : manipulatedEntities)
                        {
                            auto& transform = ecs.GetComponent<components::TransformComponent>(entity);
                            transform.m_Scale += scaleDelta;
                        }
                    }
                    break;
                }
            }

            break;
        }
    }

    void handleGizmoControlUi(
        atlas::game::scene::systems::debug::DebugAxisRenderSystem::ManipulatorType& type,
        atlas::game::scene::systems::debug::DebugAxisRenderSystem::TransformSpace& space
        )
    {
        using namespace atlas::game::scene::systems::debug;

        ImGuiWindowFlags corner =
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoTitleBar;

        const auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
        bool p_open {true};

        constexpr float c_padding = 5.0f;
        float offset = c_padding;
        if (ImGui::Begin("ManipulatorType", &p_open, corner))
        {
            if (ImGui::RadioButton("T", type == DebugAxisRenderSystem::ManipulatorType::Translate))
            {
                type = DebugAxisRenderSystem::ManipulatorType::Translate;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("R", type == DebugAxisRenderSystem::ManipulatorType::Rotation))
            {
                type = DebugAxisRenderSystem::ManipulatorType::Rotation;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("S", type == DebugAxisRenderSystem::ManipulatorType::Scale))
            {
                type = DebugAxisRenderSystem::ManipulatorType::Scale;
            }

            ImGui::SetWindowPos(ImVec2(width - ImGui::GetWindowWidth() - offset, 0), true);
            offset += ImGui::GetWindowWidth() + c_padding;
        }
        ImGui::End();

        if (ImGui::Begin("TransformSpace", &p_open, corner))
        {
            if (ImGui::RadioButton("W", space == DebugAxisRenderSystem::TransformSpace::World))
            {
                space = DebugAxisRenderSystem::TransformSpace::World;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("L", space == DebugAxisRenderSystem::TransformSpace::Local))
            {
                space = DebugAxisRenderSystem::TransformSpace::Local;
            }

            ImGui::SetWindowPos(ImVec2(width - ImGui::GetWindowWidth() - offset, 0), true);
            offset += ImGui::GetWindowWidth();
        }
        ImGui::End();
    }
}

atlas::game::scene::systems::debug::DebugAxisRenderSystem::DebugAxisRenderSystem(const bgfx::ViewId view)
    : m_View{view}
{
}

void atlas::game::scene::systems::debug::DebugAxisRenderSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    SystemBase::Initialise(ecsManager);
}

void atlas::game::scene::systems::debug::DebugAxisRenderSystem::Render(atlas::scene::EcsManager& ecs)
{
    auto [width, height] = app_host::Application::Get().GetAppDimensions();
    ImGuizmo::SetRect(0, 0, static_cast<float>(width), static_cast<float>(height));

    drawGrid();
    handleGizmoControlUi(m_ManipulatorType, m_TransformSpace);
    handleGizmos(ecs, m_View, m_ManipulatorType, m_TransformSpace);
}
