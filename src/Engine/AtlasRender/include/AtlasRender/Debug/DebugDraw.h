#pragma once

#include <bgfx/bgfx.h>
#include <bx/allocator.h>
#include <bx/bounds.h>

#include "AtlasCore/Colour.h"

namespace atlas::render::debug
{
    void initialise();

    void shutdown();

    class DebugDrawEncoderScopePush
    {
    public:
        DebugDrawEncoderScopePush();
        ~DebugDrawEncoderScopePush();
    };

    namespace debug_draw
    {
        void begin(uint16_t viewId, bool depthTestLess = true, bgfx::Encoder* encoder = nullptr);

        void end();

        void push();

        void pop();

        void setDepthTestAlways();

        void setDepthTestLess(bool depthTestLess);

        void setState(bool depthTest, bool depthWrite, bool clockwise);

        void setColor(core::Colour32 colour);

        void setLod(uint8_t lod);

        void setWireframe(bool wireframe);

        void setStipple(bool stipple, float scale = 1.0f, float offset = 0.0f);

        void setSpin(float spin);

        void setTransform(const void* mtx);

        void setTranslate(float x, float y, float z);

        void pushTransform(const void* mtx);

        void popTransform();

        void moveTo(float x, float y, float z = 0.0f);

        void moveTo(const bx::Vec3& pos);

        void lineTo(float x, float y, float z = 0.0f);

        void lineTo(const bx::Vec3& pos);

        void close();

        void draw(const bx::Aabb& aabb);

        void draw(const bx::Cylinder& cylinder);

        void draw(const bx::Capsule& capsule);

        void draw(const bx::Disk& disk);

        void draw(const bx::Obb& obb);

        void draw(const bx::Sphere& sphere);

        void draw(const bx::Triangle& triangle);

        void draw(const bx::Cone& cone);

        void drawFrustum(const void* viewProj);

        void drawAxis(float x, float y, float z, float len = 1.0f, float thickness = 0.0f);

        void drawCircle(const bx::Vec3& normal, const bx::Vec3& center, float radius, float weight = 0.0f);

        void drawQuad(const bx::Vec3& normal, const bx::Vec3& center, float size);

        void drawQuad(bgfx::TextureHandle handle, const bx::Vec3& normal, const bx::Vec3& center, float size);

        void drawCone(const bx::Vec3& from, const bx::Vec3& to, float radius);

        void drawCylinder(const bx::Vec3& from, const bx::Vec3& to, float radius);

        void drawCapsule(const bx::Vec3& from, const bx::Vec3& to, float radius);

        void drawGrid(const bx::Vec3& normal, const bx::Vec3& center, uint32_t size = 20, float step = 1.0f);

        DebugDrawEncoderScopePush createScope();
    }
}
