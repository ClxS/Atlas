#pragma once
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasRender/Types/FrameBuffer.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace atlas::game::scene::systems::rendering
{
    class PostProcessSystem final : public atlas::scene::SystemBase
    {
    public:
        explicit PostProcessSystem(bgfx::ViewId view, render::FrameBuffer& gbuffer);

        [[nodiscard]] std::string_view GetName() const override { return "PostProcessSystem"; }

        void Initialise(atlas::scene::EcsManager&) override;
        void Render(atlas::scene::EcsManager& ecs) override;

        void PerformCopy(bgfx::ViewId viewId, bgfx::TextureHandle source, bgfx::FrameBufferHandle target) const;

    private:
        enum class Scope
        {
            Interstitial,
            InputBuffer,
            OutputBuffer
        };

        bgfx::ViewId m_View;
        render::FrameBuffer& m_GBuffer;

        struct
        {
            resource::AssetPtr<render::ShaderProgram> m_Fxaa{};
            resource::AssetPtr<render::ShaderProgram> m_Vignette{};
            resource::AssetPtr<render::ShaderProgram> m_Copy{};
        } m_Programs;

        struct
        {
            bgfx::UniformHandle m_Color{BGFX_INVALID_HANDLE};
        } m_Samplers;

        struct
        {
            bgfx::UniformHandle m_FrameBufferSize{BGFX_INVALID_HANDLE};
        } m_Uniforms;

        struct
        {
            render::FrameBuffer m_FrameBuffer;
        } m_Interstitials;

        bgfx::VertexLayout m_PostProcessLayout{};
        bgfx::VertexBufferHandle m_FullScreenQuad{};
        float m_TexelHalf{};

        [[nodiscard]] bgfx::TextureHandle GetInputAsTexture(Scope target) const;
        [[nodiscard]] bgfx::FrameBufferHandle GetTargetFrameBuffer(Scope target) const;

        void PrepareFrame();
        void DoFxaa(bgfx::ViewId viewId, Scope source, Scope target) const;
        void DoVignette(bgfx::ViewId viewId, Scope source, Scope target) const;
        void DoCopy(bgfx::ViewId viewId, Scope source, Scope target) const;
    };
}
