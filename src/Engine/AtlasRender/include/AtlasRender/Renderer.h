#pragma once
#include <cstdint>
#include <functional>
#include <string>

#include <Eigen/Core>

#include "AtlasCore/MathsHelpers.h"
#include "AtlasResource/AssetPtr.h"
#include "bgfx/bgfx.h"

namespace atlas::render
{
    class ShaderProgram;
    class ModelAsset;

    struct RendererInitArgs
    {
        void* m_WindowHandle;

        int16_t m_Width;
        int16_t m_Height;

        bgfx::ViewId m_DebugGeometryView;
    };

    struct RenderTaskHandle
    {
        int32_t m_Value;
    };

    struct DrawInstance
    {
        Eigen::Vector3f m_Position;
        maths_helpers::Angle m_Rotation;
    };

    struct ShadowCaster
    {
        Eigen::Matrix4f m_LightMatrix;
        bgfx::TextureHandle m_ShadowMap;
    };

    void init(const RendererInitArgs& args);

    RenderTaskHandle addToFrameGraph(std::string_view name, std::function<void()> initiailize, std::function<void()> callback, std::vector<RenderTaskHandle> dependentTasks = {});
    RenderTaskHandle addToFrameGraph(std::string_view name, std::function<void()> callback, std::vector<RenderTaskHandle> dependentTasks = {});
    RenderTaskHandle addToFrameGraph_oneOff(std::string_view name, std::function<void()> callback, std::vector<RenderTaskHandle> dependentTasks = {});

    void setShadowCaster(uint8_t shadowIndex, ShadowCaster shadow);

    void draw(
        bgfx::ViewId viewId,
        bgfx::VertexBufferHandle vertexBuffer,
        const resource::AssetPtr<ShaderProgram>& program,
        const Eigen::Matrix4f& transform,
        uint8_t flags = BGFX_DISCARD_ALL);

    void draw(
        bgfx::ViewId viewId,
        const bgfx::TransientVertexBuffer& vertexBuffer,
        const resource::AssetPtr<ShaderProgram>& program,
        const Eigen::Matrix4f& transform,
        uint8_t flags = BGFX_DISCARD_ALL);

    void draw(
        bgfx::ViewId viewId,
        bgfx::VertexBufferHandle vertexBuffer,
        bgfx::IndexBufferHandle indexBuffer,
        const resource::AssetPtr<ShaderProgram>& program,
        const Eigen::Matrix4f& transform,
        uint8_t flags = BGFX_DISCARD_ALL);

    void draw(
        bgfx::ViewId viewId,
        const resource::AssetPtr<ModelAsset>& model,
        const resource::AssetPtr<ShaderProgram>& program,
        const Eigen::Matrix4f& transform,
        uint64_t state,
        uint32_t fstencil,
        uint32_t bstencil,
        uint8_t flags = BGFX_DISCARD_ALL);

    void drawInstanced(
        bgfx::ViewId viewId,
        const resource::AssetPtr<ModelAsset>& model,
        const resource::AssetPtr<ShaderProgram>& program,
        const std::vector<Eigen::Matrix4f>& transforms,
        uint64_t state = BGFX_STATE_DEFAULT,
        uint32_t fstencil = BGFX_STENCIL_NONE,
        uint32_t bstencil = BGFX_STENCIL_NONE,
        uint8_t flags = BGFX_DISCARD_ALL);

    void sync();
}
