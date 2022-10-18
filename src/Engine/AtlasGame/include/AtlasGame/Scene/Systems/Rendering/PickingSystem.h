#pragma once
#include <bgfx/bgfx.h>

#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasRender/Types/FrameBuffer.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

#include "AtlasResource/AssetPtr.h"

namespace atlas::game::scene::systems::rendering
{
    class ModelRenderSystem;

    class PickingSystem final : public atlas::scene::SystemBase
    {
    public:
        explicit PickingSystem(bgfx::ViewId pickingSystemView, uint8_t pickableRenderMask, uint16_t pickFrameWidth = 512, uint16_t pickFrameHeight = 512);

        [[nodiscard]] std::string_view GetName() const override { return "PickingSystem"; }

        void Initialise(atlas::scene::EcsManager& ecs) override;
        void Update(atlas::scene::EcsManager& ecs) override;
        void Render(atlas::scene::EcsManager& ecs) override;

        bgfx::FrameBufferHandle GetPickingFrameBuffer() const { return m_PickingFrame.GetHandle(); }

    private:
        bgfx::ViewId m_View;
        uint8_t m_PickableRenderMask;

        uint16_t m_PickFrameWidth{512};
        uint16_t m_PickFrameHeight{512};

        std::unique_ptr<ModelRenderSystem> m_ModelRenderSystem;

        struct
        {
            bgfx::UniformHandle m_ObjectId{BGFX_INVALID_HANDLE};
        } m_Uniforms;

        struct
        {
            resource::AssetPtr<render::ShaderProgram> m_PickingObjectShader;
        } m_Programs;

        render::FrameBuffer m_PickingFrame;
    };
}
