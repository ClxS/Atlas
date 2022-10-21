#pragma once
#include <future>
#include <bgfx/bgfx.h>

#include "AtlasRender/BgfxHandle.h"
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
            const std::array<bgfx::ViewId, 2>& pickingViews,
            uint8_t pickableRenderMask,
            uint16_t pickFrameWidth = c_defaultPickingBufferSize,
            uint16_t pickFrameHeight = c_defaultPickingBufferSize);

        [[nodiscard]] std::string_view GetName() const override { return "PickingSystem"; }

        void Initialise(atlas::scene::EcsManager& ecs) override;
        void Update(atlas::scene::EcsManager& ecs) override;
        void Render(atlas::scene::EcsManager& ecs) override;

        bgfx::FrameBufferHandle GetPickingFrameBuffer() const { return m_PickingFrame.GetHandle(); }

        std::future<atlas::scene::EntityId> RequestPick(int32_t x, int32_t y, std::vector<atlas::scene::EntityId> exclusions = {});

    private:
        struct PickRequest
        {
            int32_t m_X;
            int32_t m_Y;
            std::promise<atlas::scene::EntityId> m_Result;
            std::vector<atlas::scene::EntityId> m_Exclusions;
            bool m_IsComplete = false;
        };

        bgfx::ViewId m_PickingBufferView;
        bgfx::ViewId m_PickingBlitView;

        uint8_t m_PickableRenderMask;

        uint16_t m_PickFrameWidth{c_defaultPickingBufferSize};
        uint16_t m_PickFrameHeight{c_defaultPickingBufferSize};

        std::unique_ptr<rendering::ModelRenderSystem> m_ModelRenderSystem;

        struct
        {
            render::BgfxHandle<bgfx::UniformHandle> m_ObjectId{BGFX_INVALID_HANDLE};
        } m_Uniforms;

        struct
        {
            resource::AssetPtr<render::ShaderProgram> m_PickingObjectShader;
        } m_Programs;

        render::FrameBuffer m_PickingFrame;
        render::Texture m_BlittingFrame;

        std::vector<uint8_t> m_PickingFrameData;

        std::vector<std::unique_ptr<PickRequest>> m_PickRequests;
    };
}
