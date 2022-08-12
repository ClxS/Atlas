#pragma once
#include "AtlasResource/ResourceLoader.h"
#include "bgfx/bgfx.h"
#include "RmlUi/Core/RenderInterface.h"

namespace atlas::render
{
    class TextureAsset;
    class ShaderProgram;
}

namespace atlas::ui::rml
{
    class RmlRenderInterface final : public Rml::RenderInterface
    {
    public:
        explicit RmlRenderInterface(bgfx::ViewId viewId);

        Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture) override;

        void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometryHandle, const Rml::Vector2f& translation) override;

        void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometryHandle) override;

        void RenderGeometry(
            Rml::Vertex* vertices,
            const int numVertices,
            int* indices,
            const int numIndices,
            const Rml::TextureHandle texture,
            const Rml::Vector2f& translation) override;

        void SetScissorRegion(int x, int y, int width, int height) override;

        bool LoadTexture(Rml::TextureHandle& textureHandle, Rml::Vector2i& textureDimensions, const Rml::String& source) override;

        bool GenerateTexture(Rml::TextureHandle& textureHandle, const Rml::byte* source, const Rml::Vector2i& sourceDimensions) override;

        void ReleaseTexture(Rml::TextureHandle texture) override;

        void EnableScissorRegion(bool enable) override;

    private:
        void SetTransformUniform(const Rml::Vector2f& translation) const;

        struct Parameters
        {
            bgfx::ViewId m_RenderViewId;
        } m_Params{};

        struct CompiledGeometry
        {
            bgfx::VertexBufferHandle m_VertexBuffer{BGFX_INVALID_HANDLE};
            bgfx::IndexBufferHandle m_IndexBuffer{BGFX_INVALID_HANDLE};
            Rml::TextureHandle m_Texture{};
        };

        struct Texture
        {
            resource::AssetPtr<render::TextureAsset> m_Asset{};
            bgfx::TextureHandle m_Handle{};
            uint32_t m_Width{};
            uint32_t m_Height{};
        };

        struct
        {
            bgfx::UniformHandle m_Transform{BGFX_INVALID_HANDLE};
            bgfx::UniformHandle m_Sampler{BGFX_INVALID_HANDLE};
        } m_Uniforms;

        Eigen::Vector4i m_ScissorRegion;
        resource::AssetPtr<render::ShaderProgram> m_TexturedProgram;
        resource::AssetPtr<render::ShaderProgram> m_UntexturedProgram;
        bgfx::VertexLayout m_RmlVertexLayout{};

        Rml::CompiledGeometryHandle m_CompiledGeometryCounter{};
        std::unordered_map<Rml::CompiledGeometryHandle, CompiledGeometry> m_CompiledGeometry;

        Rml::TextureHandle m_TextureCounter{};
        std::unordered_map<Rml::TextureHandle, Texture> m_Textures;
    };


}
