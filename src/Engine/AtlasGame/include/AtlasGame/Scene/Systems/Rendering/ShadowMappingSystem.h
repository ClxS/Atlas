#pragma once
#include "ModelRenderSystem.h"
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasRender/Types/FrameBuffer.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace atlas::game::scene::systems::rendering
{
    class ShadowMappingSystem final : public ModelRenderSystem
    {
    public:
        explicit ShadowMappingSystem(bgfx::ViewId shadowViewId, uint8_t shadowCasterRenderMask, uint16_t shadowMapWidth = 2048, uint16_t shadowMapHeight = 2048);

        [[nodiscard]] std::string_view GetName() const override { return "ShadowMappingSystem"; }

        void Initialise(atlas::scene::EcsManager&) override;
        void Render(atlas::scene::EcsManager& ecs) override;

    private:
        bgfx::ViewId m_ShadowViewId;
        uint16_t m_ShadowMapWidth{512};
        uint16_t m_ShadowMapHeight{512};

        struct
        {
            resource::AssetPtr<render::ShaderProgram> m_ShadowMap;
        } m_Programs;

        bgfx::FrameBufferHandle m_ShadowMapFrameBuffer{BGFX_INVALID_HANDLE};
    };
}
