#pragma once
#include <bgfx/bgfx.h>

#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasRender/Types/FrameBuffer.h"
#include "AtlasRender/Types/Texture.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

#include "AtlasResource/AssetPtr.h"
#include "AtlasScene/ECS/Entity.h"

namespace atlas::game::scene::systems::rendering
{
    class ModelRenderSystem;
}

namespace atlas::game::scene::systems::interactivity
{
    constexpr uint16_t c_defaultPickingBufferSize = 32;

    class PickingSystem final : public atlas::scene::SystemBase
    {
    public:
        explicit PickingSystem(
            bgfx::ViewId pickingBufferView,
            bgfx::ViewId pickingBlitView,
            uint8_t pickableRenderMask,
            uint16_t pickFrameWidth = c_defaultPickingBufferSize,
            uint16_t pickFrameHeight = c_defaultPickingBufferSize);

        [[nodiscard]] std::string_view GetName() const override { return "PickingSystem"; }

        void Initialise(atlas::scene::EcsManager& ecs) override;
        void Update(atlas::scene::EcsManager& ecs) override;
        void Render(atlas::scene::EcsManager& ecs) override;

        bgfx::FrameBufferHandle GetPickingFrameBuffer() const { return m_PickingFrame.GetHandle(); }

        atlas::scene::EntityId GetHoveredEntity() const { return m_HoveredEntity; }

    private:
        bgfx::ViewId m_PickingBufferView;
        bgfx::ViewId m_PickingBlitView;

        uint8_t m_PickableRenderMask;

        uint16_t m_PickFrameWidth{c_defaultPickingBufferSize};
        uint16_t m_PickFrameHeight{c_defaultPickingBufferSize};

        std::unique_ptr<rendering::ModelRenderSystem> m_ModelRenderSystem;

        struct
        {
            bgfx::UniformHandle m_ObjectId{BGFX_INVALID_HANDLE};
        } m_Uniforms;

        struct
        {
            resource::AssetPtr<render::ShaderProgram> m_PickingObjectShader;
        } m_Programs;

        render::FrameBuffer m_PickingFrame;
        render::Texture m_BlittingFrame;

        std::vector<uint8_t> m_PickingFrameData;

        atlas::scene::EntityId m_HoveredEntity{atlas::scene::EntityId::Invalid()};
    };
}
