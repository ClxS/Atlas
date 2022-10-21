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
            const std::array<bgfx::ViewId, 4>& viewIds,
            bgfx::FrameBufferHandle targetFrameBuffer)
            : m_SilhouetteView{viewIds[0]}
            , m_BlurredView{viewIds[1]}
            , m_OutlineFormationView{viewIds[2]}
            , m_OutlineBlitView{viewIds[3]}
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

        bgfx::ViewId m_SilhouetteView;
        bgfx::ViewId m_BlurredView;
        bgfx::ViewId m_OutlineFormationView;
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
