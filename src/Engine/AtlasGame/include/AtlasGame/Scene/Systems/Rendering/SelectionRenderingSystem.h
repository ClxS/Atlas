#pragma once
#include <bgfx/bgfx.h>

#include "AtlasRender/BgfxHandle.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasRender/Types/FrameBuffer.h"
#include "AtlasResource/AssetPtr.h"

namespace atlas::game::scene::systems::rendering
{
    class ModelRenderSystem;

    class SelectionRenderingSystem final : public atlas::scene::SystemBase
    {
    public:
        SelectionRenderingSystem(
            const bgfx::ViewId stencilRenderView,
            const bgfx::ViewId outlineRenderView,
            const bgfx::ViewId outlineBlitView,
            bgfx::FrameBufferHandle targetFrameBuffer)
            : m_SilhoutteView{stencilRenderView}
            , m_BlurredView{outlineRenderView}
            , m_OutlineBlitView{outlineBlitView}
            , m_TargetFrameBuffer{targetFrameBuffer}
        {
        }

        [[nodiscard]] std::string_view GetName() const override { return "SelectionRenderingSystem"; }

        void Initialise(atlas::scene::EcsManager&) override;
        void Render(atlas::scene::EcsManager& ecs) override;

    private:
        void RenderSelectedObjects(
            atlas::scene::EcsManager& ecs,
            uint8_t stencilValue) const;
        void RenderOutline() const;
        void ApplyOutline();

        bgfx::ViewId m_SilhoutteView;
        bgfx::ViewId m_BlurredView;
        bgfx::ViewId m_OutlineBlitView;
        bgfx::FrameBufferHandle m_TargetFrameBuffer;

        struct
        {
            resource::AssetPtr<render::ShaderProgram> m_StencilUpdate;
            resource::AssetPtr<render::ShaderProgram> m_CreateBlur;
            resource::AssetPtr<render::ShaderProgram> m_Clear;
            resource::AssetPtr<render::ShaderProgram> m_Copy;
        } m_Programs;

        struct
        {
            render::BgfxHandle<bgfx::UniformHandle> m_Color{BGFX_INVALID_HANDLE};
        } m_Samplers;

        struct
        {
            render::FrameBuffer m_Silhouette;
            render::FrameBuffer m_BlurredSilhouette;
        } m_Buffers;

        bgfx::VertexLayout m_OutlineVertexLayout{};
        render::BgfxHandle<bgfx::VertexBufferHandle> m_FullScreenQuad{};
    };
}
