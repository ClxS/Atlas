/*
 * Copyright 2011-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "AtlasRenderPCH.h"
#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include "debugdraw.h"

#include <bx/debug.h>
#include <bx/handlealloc.h>
#include <bx/math.h>
#include <bx/mutex.h>
#include <bx/uint32_t.h>

#include "AtlasCore/Colour.h"

#ifndef DEBUG_DRAW_CONFIG_MAX_GEOMETRY
#	define DEBUG_DRAW_CONFIG_MAX_GEOMETRY 256
#endif // DEBUG_DRAW_CONFIG_MAX_GEOMETRY

namespace atlas::render::debug
{
    struct Axis
    {
        enum Enum
        {
            X,
            Y,
            Z,

            Count
        };
    };

    struct DdVertex
    {
        float m_X, m_Y, m_Z;
    };

    struct SpriteHandle
    {
        uint16_t m_Idx;
    };

    inline bool isValid(const SpriteHandle handle) { return handle.m_Idx != UINT16_MAX; }

    struct GeometryHandle
    {
        uint16_t m_Idx;
    };

    inline bool isValid(const GeometryHandle handle) { return handle.m_Idx != UINT16_MAX; }


    ///
    SpriteHandle ddCreateSprite(uint16_t width, uint16_t height, const void* data);

    ///
    void ddDestroy(SpriteHandle handle);

    ///
    GeometryHandle ddCreateGeometry(uint32_t numVertices, const DdVertex* vertices, uint32_t numIndices = 0,
                                    const void* indices = nullptr, bool index32 = false);

    ///
    void ddDestroy(GeometryHandle handle);
}

bool checkAvailTransientBuffers(const uint32_t numVertices, const bgfx::VertexLayout& layout, const uint32_t numIndices)
{
    return numVertices == getAvailTransientVertexBuffer(numVertices, layout)
        && (0 == numIndices || numIndices == bgfx::getAvailTransientIndexBuffer(numIndices));
}

struct Pack2D
{
    uint16_t m_X;
    uint16_t m_Y;
    uint16_t m_Width;
    uint16_t m_Height;
};

struct PackCube
{
    Pack2D m_Rect;
    uint8_t m_Side;
};

template <uint16_t NumBlocks>
class RectPackCubeT;

template <uint16_t TNumBlocks>
class RectPack2Dt
{
public:
    RectPack2Dt() = delete;
    RectPack2Dt(const uint16_t width, const uint16_t height)
    {
        Reset(width, height);
    }

    void Reset(const uint16_t width, const uint16_t height)
    {
        m_Bw = width / 64;
        m_Bh = height / TNumBlocks;
        bx::memSet(m_Mem, 0xff, sizeof(m_Mem));
    }

    bool Find(const uint16_t inWidth, const uint16_t inHeight, Pack2D& pack)
    {
        const auto width = bx::min<uint16_t>(64, (inWidth + m_Bw - 1) / m_Bw);
        const auto height = bx::min<uint16_t>(TNumBlocks, (inHeight + m_Bh - 1) / m_Bh);
        const uint16_t numX = 64 - width;
        const uint16_t numY = TNumBlocks - height;

        const uint64_t scan = width == 64 ? UINT64_MAX : (UINT64_C(1) << width) - 1;

        for (uint16_t startY = 0; startY <= numY; ++startY)
        {
            const uint64_t mem = m_Mem[startY];
            const auto ntz = static_cast<uint16_t>(bx::uint64_cnttz(mem));
            uint64_t mask = scan << ntz;

            for (uint16_t xx = ntz; xx <= numX; ++xx, mask <<= 1)
            {
                uint16_t yy = startY;
                if ((mem & mask) == mask)
                {
                    const uint16_t endY = startY + height;
                    while (yy < endY && (m_Mem[yy] & mask) == mask)
                    {
                        ++yy;
                    }

                    if (yy == endY)
                    {
                        uint64_t invMask = ~mask;
                        for (yy = startY; yy < endY; ++yy)
                        {
                            m_Mem[yy] &= invMask;
                        }

                        pack.m_X = xx * m_Bw;
                        pack.m_Y = startY * m_Bh;
                        pack.m_Width = width * m_Bw;
                        pack.m_Height = height * m_Bh;
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void Clear(const Pack2D& pack)
    {
        const auto startX = bx::min<uint16_t>(63, pack.m_X / m_Bw);
        const auto startY = bx::min<uint16_t>(TNumBlocks - 1, pack.m_Y / m_Bh);
        const auto endX = bx::min<uint16_t>(64, (pack.m_Width + m_Bw - 1) / m_Bw + startX);
        const auto endY = bx::min<uint16_t>(TNumBlocks, (pack.m_Height + m_Bh - 1) / m_Bh + startY);
        const uint16_t width = endX - startX;

        const uint64_t mask = (width == 64 ? UINT64_MAX : (UINT64_C(1) << width) - 1) << startX;

        for (uint16_t yy = startY; yy < endY; ++yy)
        {
            m_Mem[yy] |= mask;
        }
    }

private:
    friend class RectPackCubeT<TNumBlocks>;

    uint64_t m_Mem[TNumBlocks]{};
    uint16_t m_Bw{};
    uint16_t m_Bh{};
};

template <uint16_t TNumBlocks>
class RectPackCubeT
{
public:
    RectPackCubeT() = delete;
    explicit RectPackCubeT(const uint16_t side)
    {
        Reset(side);
    }

    void Reset(uint16_t side)
    {
        for (uint8_t ii = 0; ii < 6; ++ii)
        {
            m_Mru[ii] = ii;
            m_Ra[ii].reset(side, side);
        }
    }

    bool Find(uint16_t width, uint16_t height, PackCube& pack)
    {
        for (uint32_t ii = 0; ii < 6; ++ii)
        {
            uint8_t side = m_Mru[ii];
            bool found = m_Ra[side].Find(width, height, pack.m_Rect);

            if (found)
            {
                pack.m_Side = side;
                m_Mru[ii] = m_Mru[0];
                m_Mru[0] = side;
                return true;
            }
        }

        return false;
    }

    void Clear(const PackCube& pack)
    {
        uint8_t side = pack.m_Side;

        uint32_t ii = 0;
        for (; ii < 6 && m_Mru[ii] != side; ++ii)
        {
        }

        m_Mru[ii] = m_Mru[0];
        m_Mru[0] = side;

        m_Ra[side].clear(pack.m_Rect);
    }

private:
    RectPack2Dt<TNumBlocks> m_Ra[6];
    uint8_t m_Mru[6];
};

struct DebugVertex
{
    float m_X;
    float m_Y;
    float m_Z;
    float m_Len;
    uint32_t m_Abgr;

    static void Init()
    {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 1, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    }

    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout DebugVertex::ms_layout;

struct DebugUvVertex
{
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;
    uint32_t m_abgr;

    static void init()
    {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    }

    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout DebugUvVertex::ms_layout;

struct DebugShapeVertex
{
    float m_x;
    float m_y;
    float m_z;
    uint8_t m_indices[4];

    static void init()
    {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Uint8)
            .end();
    }

    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout DebugShapeVertex::ms_layout;

struct DebugMeshVertex
{
    float m_x;
    float m_y;
    float m_z;

    static void init()
    {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .end();
    }

    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout DebugMeshVertex::ms_layout;

static DebugShapeVertex s_quadVertices[4] =
{
    {-1.0f, 0.0f, 1.0f, {0, 0, 0, 0}},
    {1.0f, 0.0f, 1.0f, {0, 0, 0, 0}},
    {-1.0f, 0.0f, -1.0f, {0, 0, 0, 0}},
    {1.0f, 0.0f, -1.0f, {0, 0, 0, 0}},

};

static const uint16_t s_quadIndices[6] =
{
    0, 1, 2,
    1, 3, 2,
};

static DebugShapeVertex s_cubeVertices[8] =
{
    {-1.0f, 1.0f, 1.0f, {0, 0, 0, 0}},
    {1.0f, 1.0f, 1.0f, {0, 0, 0, 0}},
    {-1.0f, -1.0f, 1.0f, {0, 0, 0, 0}},
    {1.0f, -1.0f, 1.0f, {0, 0, 0, 0}},
    {-1.0f, 1.0f, -1.0f, {0, 0, 0, 0}},
    {1.0f, 1.0f, -1.0f, {0, 0, 0, 0}},
    {-1.0f, -1.0f, -1.0f, {0, 0, 0, 0}},
    {1.0f, -1.0f, -1.0f, {0, 0, 0, 0}},
};

static const uint16_t s_cubeIndices[36] =
{
    0, 1, 2, // 0
    1, 3, 2,
    4, 6, 5, // 2
    5, 6, 7,
    0, 2, 4, // 4
    4, 2, 6,
    1, 5, 3, // 6
    5, 7, 3,
    0, 4, 1, // 8
    4, 5, 1,
    2, 3, 6, // 10
    6, 3, 7,
};

static constexpr uint8_t c_circleLod[] =
{
    37,
    29,
    23,
    17,
    11,
};

static uint8_t getCircleLod(uint8_t lod)
{
    lod = lod > BX_COUNTOF(c_circleLod) - 1 ? BX_COUNTOF(c_circleLod) - 1 : lod;
    return c_circleLod[lod];
}

static void circle(float* out, const float angle)
{
    const float sa = bx::sin(angle);
    const float ca = bx::cos(angle);
    out[0] = sa;
    out[1] = ca;
}

static void Squircle(float* out, const float angle)
{
    const float sa = bx::sin(angle);
    const float ca = bx::cos(angle);
    out[0] = bx::sqrt(bx::abs(sa)) * bx::sign(sa);
    out[1] = bx::sqrt(bx::abs(ca)) * bx::sign(ca);
}

uint32_t genSphere(
    const uint8_t subdiv0,
    void* pos0 = nullptr,
    const uint16_t posStride0 = 0,
    void* normals0 = nullptr,
    const uint16_t normalStride0 = 0)
{
    if (nullptr != pos0)
    {
        struct Gen
        {
            Gen(void* _pos, uint16_t _posStride, void* _normals, uint16_t _normalStride, uint8_t _subdiv)
                : m_pos(static_cast<uint8_t*>(_pos))
                  , m_normals(static_cast<uint8_t*>(_normals))
                  , m_posStride(_posStride)
                  , m_normalStride(_normalStride)
            {
                static constexpr float c_scale = 1.0f;
                static constexpr float c_golden = 1.6180339887f;
                static const float c_len = bx::sqrt(c_golden * c_golden + 1.0f);
                static const float c_ss = 1.0f / c_len * c_scale;
                static const float c_ll = c_ss * c_golden;

                static const bx::Vec3 vv[] =
                {
                    {-c_ll, 0.0f, -c_ss},
                    {c_ll, 0.0f, -c_ss},
                    {c_ll, 0.0f, c_ss},
                    {-c_ll, 0.0f, c_ss},

                    {-c_ss, c_ll, 0.0f},
                    {c_ss, c_ll, 0.0f},
                    {c_ss, -c_ll, 0.0f},
                    {-c_ss, -c_ll, 0.0f},

                    {0.0f, -c_ss, c_ll},
                    {0.0f, c_ss, c_ll},
                    {0.0f, c_ss, -c_ll},
                    {0.0f, -c_ss, -c_ll},
                };

                m_numVertices = 0;

                triangle(vv[0], vv[4], vv[3], c_scale, _subdiv);
                triangle(vv[0], vv[10], vv[4], c_scale, _subdiv);
                triangle(vv[4], vv[10], vv[5], c_scale, _subdiv);
                triangle(vv[5], vv[10], vv[1], c_scale, _subdiv);
                triangle(vv[5], vv[1], vv[2], c_scale, _subdiv);
                triangle(vv[5], vv[2], vv[9], c_scale, _subdiv);
                triangle(vv[5], vv[9], vv[4], c_scale, _subdiv);
                triangle(vv[3], vv[4], vv[9], c_scale, _subdiv);

                triangle(vv[0], vv[3], vv[7], c_scale, _subdiv);
                triangle(vv[0], vv[7], vv[11], c_scale, _subdiv);
                triangle(vv[11], vv[7], vv[6], c_scale, _subdiv);
                triangle(vv[11], vv[6], vv[1], c_scale, _subdiv);
                triangle(vv[1], vv[6], vv[2], c_scale, _subdiv);
                triangle(vv[2], vv[6], vv[8], c_scale, _subdiv);
                triangle(vv[8], vv[6], vv[7], c_scale, _subdiv);
                triangle(vv[8], vv[7], vv[3], c_scale, _subdiv);

                triangle(vv[0], vv[11], vv[10], c_scale, _subdiv);
                triangle(vv[1], vv[10], vv[11], c_scale, _subdiv);
                triangle(vv[2], vv[8], vv[9], c_scale, _subdiv);
                triangle(vv[3], vv[9], vv[8], c_scale, _subdiv);
            }

            void addVert(const bx::Vec3& _v)
            {
                store(m_pos, _v);
                m_pos += m_posStride;

                if (nullptr != m_normals)
                {
                    const bx::Vec3 normal = normalize(_v);
                    store(m_normals, normal);

                    m_normals += m_normalStride;
                }

                m_numVertices++;
            }

            void triangle(const bx::Vec3& _v0, const bx::Vec3& _v1, const bx::Vec3& _v2, float _scale, uint8_t _subdiv)
            {
                if (0 == _subdiv)
                {
                    addVert(_v0);
                    addVert(_v1);
                    addVert(_v2);
                }
                else
                {
                    const bx::Vec3 v01 = mul(normalize(add(_v0, _v1)), _scale);
                    const bx::Vec3 v12 = mul(normalize(add(_v1, _v2)), _scale);
                    const bx::Vec3 v20 = mul(normalize(add(_v2, _v0)), _scale);

                    --_subdiv;
                    triangle(_v0, v01, v20, _scale, _subdiv);
                    triangle(_v1, v12, v01, _scale, _subdiv);
                    triangle(_v2, v20, v12, _scale, _subdiv);
                    triangle(v01, v12, v20, _scale, _subdiv);
                }
            }

            uint8_t* m_pos;
            uint8_t* m_normals;
            uint16_t m_posStride;
            uint16_t m_normalStride;
            uint32_t m_numVertices;
        } gen(pos0, posStride0, normals0, normalStride0, subdiv0);
    }

    uint32_t numVertices = 20 * 3 * bx::uint32_max(1, static_cast<uint32_t>(bx::pow(4.0f, subdiv0)));
    return numVertices;
}

bx::Vec3 getPoint(atlas::render::debug::Axis::Enum _axis, float _x, float _y)
{
    switch (_axis)
    {
    case atlas::render::debug::Axis::X: return {0.0f, _x, _y};
    case atlas::render::debug::Axis::Y: return {_y, 0.0f, _x};
    default: break;
    }

    return {_x, _y, 0.0f};
}

#include "fs_debugdraw_fill.bin.h"
#include "fs_debugdraw_fill_lit.bin.h"
#include "fs_debugdraw_fill_texture.bin.h"
#include "fs_debugdraw_lines.bin.h"
#include "fs_debugdraw_lines_stipple.bin.h"
#include "vs_debugdraw_fill.bin.h"
#include "vs_debugdraw_fill_lit.bin.h"
#include "vs_debugdraw_fill_lit_mesh.bin.h"
#include "vs_debugdraw_fill_mesh.bin.h"
#include "vs_debugdraw_fill_texture.bin.h"
#include "vs_debugdraw_lines.bin.h"
#include "vs_debugdraw_lines_stipple.bin.h"

static const bgfx::EmbeddedShader s_embeddedShaders[] =
{
    BGFX_EMBEDDED_SHADER(vs_debugdraw_lines),
    BGFX_EMBEDDED_SHADER(fs_debugdraw_lines),
    BGFX_EMBEDDED_SHADER(vs_debugdraw_lines_stipple),
    BGFX_EMBEDDED_SHADER(fs_debugdraw_lines_stipple),
    BGFX_EMBEDDED_SHADER(vs_debugdraw_fill),
    BGFX_EMBEDDED_SHADER(vs_debugdraw_fill_mesh),
    BGFX_EMBEDDED_SHADER(fs_debugdraw_fill),
    BGFX_EMBEDDED_SHADER(vs_debugdraw_fill_lit),
    BGFX_EMBEDDED_SHADER(vs_debugdraw_fill_lit_mesh),
    BGFX_EMBEDDED_SHADER(fs_debugdraw_fill_lit),
    BGFX_EMBEDDED_SHADER(vs_debugdraw_fill_texture),
    BGFX_EMBEDDED_SHADER(fs_debugdraw_fill_texture),

    BGFX_EMBEDDED_SHADER_END()
};

#define SPRITE_TEXTURE_SIZE 1024

template <uint16_t MaxHandlesT = 256, uint16_t TextureSizeT = 1024>
struct SpriteT
{
    SpriteT()
        : m_ra(TextureSizeT, TextureSizeT)
    {
    }

    atlas::render::debug::SpriteHandle create(uint16_t _width, uint16_t _height)
    {
        bx::MutexScope lock(m_lock);

        atlas::render::debug::SpriteHandle handle = {bx::kInvalidHandle};

        if (m_handleAlloc.getNumHandles() < m_handleAlloc.getMaxHandles())
        {
            Pack2D pack;
            if (m_ra.Find(_width, _height, pack))
            {
                handle.m_Idx = m_handleAlloc.alloc();

                if (isValid(handle))
                {
                    m_pack[handle.m_Idx] = pack;
                }
                else
                {
                    m_ra.Clear(pack);
                }
            }
        }

        return handle;
    }

    void destroy(atlas::render::debug::SpriteHandle _sprite)
    {
        const Pack2D& pack = m_pack[_sprite.m_Idx];
        m_ra.Clear(pack);
        m_handleAlloc.free(_sprite.m_Idx);
    }

    const Pack2D& get(atlas::render::debug::SpriteHandle _sprite) const
    {
        return m_pack[_sprite.m_Idx];
    }

    bx::Mutex m_lock;
    bx::HandleAllocT<MaxHandlesT> m_handleAlloc;
    Pack2D m_pack[MaxHandlesT];
    RectPack2Dt<256> m_ra;
};

template <uint16_t MaxHandlesT = DEBUG_DRAW_CONFIG_MAX_GEOMETRY>
struct GeometryT
{
    GeometryT()
    {
    }

    atlas::render::debug::GeometryHandle create(uint32_t _numVertices, const atlas::render::debug::DdVertex* _vertices,
                                                uint32_t _numIndices, const void* _indices, bool _index32)
    {
        BX_UNUSED(_numVertices, _vertices, _numIndices, _indices, _index32);

        atlas::render::debug::GeometryHandle handle;
        {
            bx::MutexScope lock(m_lock);
            handle = {m_handleAlloc.alloc()};
        }

        if (isValid(handle))
        {
            Geometry& geometry = m_geometry[handle.m_Idx];
            geometry.m_vbh = createVertexBuffer(
                bgfx::copy(_vertices, _numVertices * sizeof(atlas::render::debug::DdVertex))
                , DebugMeshVertex::ms_layout
            );

            geometry.m_topologyNumIndices[0] = _numIndices;
            geometry.m_topologyNumIndices[1] = topologyConvert(
                bgfx::TopologyConvert::TriListToLineList
                , nullptr
                , 0
                , _indices
                , _numIndices
                , _index32
            );

            const uint32_t indexSize = _index32 ? sizeof(uint32_t) : sizeof(uint16_t);

            const uint32_t numIndices = 0
                + geometry.m_topologyNumIndices[0]
                + geometry.m_topologyNumIndices[1];
            const bgfx::Memory* mem = bgfx::alloc(numIndices * indexSize);
            uint8_t* indexData = mem->data;

            bx::memCopy(indexData, _indices, _numIndices * indexSize);
            bgfx::topologyConvert(
                bgfx::TopologyConvert::TriListToLineList
                , &indexData[geometry.m_topologyNumIndices[0] * indexSize]
                , geometry.m_topologyNumIndices[1] * indexSize
                , _indices
                , _numIndices
                , _index32
            );

            geometry.m_ibh = createIndexBuffer(
                mem
                , _index32 ? BGFX_BUFFER_INDEX32 : BGFX_BUFFER_NONE
            );
        }

        return handle;
    }

    void destroy(atlas::render::debug::GeometryHandle _handle)
    {
        bx::MutexScope lock(m_lock);
        Geometry& geometry = m_geometry[_handle.m_Idx];
        bgfx::destroy(geometry.m_vbh);
        bgfx::destroy(geometry.m_ibh);

        m_handleAlloc.free(_handle.m_Idx);
    }

    struct Geometry
    {
        Geometry()
        {
            m_vbh.idx = bx::kInvalidHandle;
            m_ibh.idx = bx::kInvalidHandle;
            m_topologyNumIndices[0] = 0;
            m_topologyNumIndices[1] = 0;
        }

        bgfx::VertexBufferHandle m_vbh;
        bgfx::IndexBufferHandle m_ibh;
        uint32_t m_topologyNumIndices[2];
    };

    bx::Mutex m_lock;
    bx::HandleAllocT<MaxHandlesT> m_handleAlloc;
    Geometry m_geometry[MaxHandlesT];
};

struct Attrib
{
    uint64_t m_state;
    float m_offset;
    float m_scale;
    float m_spin;
    uint32_t m_abgr;
    bool m_stipple;
    bool m_wireframe;
    uint8_t m_lod;
};

struct Program
{
    enum Enum
    {
        Lines,
        LinesStipple,
        Fill,
        FillMesh,
        FillLit,
        FillLitMesh,
        FillTexture,

        Count
    };
};

struct DebugMesh
{
    enum Enum
    {
        Sphere0,
        Sphere1,
        Sphere2,
        Sphere3,

        Cone0,
        Cone1,
        Cone2,
        Cone3,

        Cylinder0,
        Cylinder1,
        Cylinder2,
        Cylinder3,

        Capsule0,
        Capsule1,
        Capsule2,
        Capsule3,

        Quad,

        Cube,

        Count,

        SphereMaxLod = Sphere3 - Sphere0,
        ConeMaxLod = Cone3 - Cone0,
        CylinderMaxLod = Cylinder3 - Cylinder0,
        CapsuleMaxLod = Capsule3 - Capsule0,
    };

    uint32_t m_startVertex;
    uint32_t m_numVertices;
    uint32_t m_startIndex[2];
    uint32_t m_numIndices[2];
};

using Sprite = SpriteT<256, SPRITE_TEXTURE_SIZE>;
using Geometry = GeometryT<DEBUG_DRAW_CONFIG_MAX_GEOMETRY>;

struct DebugDrawShared
{
    void init()
    {
        static bx::DefaultAllocator allocator;
        m_allocator = &allocator;

        DebugVertex::Init();
        DebugUvVertex::init();
        DebugShapeVertex::init();
        DebugMeshVertex::init();

        bgfx::RendererType::Enum type = bgfx::getRendererType();

        m_program[Program::Lines] = createProgram(
            createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_lines")
            , createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_lines")
            , true
        );

        m_program[Program::LinesStipple] = createProgram(
            createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_lines_stipple")
            , createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_lines_stipple")
            , true
        );

        m_program[Program::Fill] = createProgram(
            createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_fill")
            , createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_fill")
            , true
        );

        m_program[Program::FillMesh] = createProgram(
            createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_fill_mesh")
            , createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_fill")
            , true
        );

        m_program[Program::FillLit] = createProgram(
            createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_fill_lit")
            , createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_fill_lit")
            , true
        );

        m_program[Program::FillLitMesh] = createProgram(
            createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_fill_lit_mesh")
            , createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_fill_lit")
            , true
        );

        m_program[Program::FillTexture] = createProgram(
            createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_fill_texture")
            , createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_fill_texture")
            , true
        );

        u_params = createUniform("u_params", bgfx::UniformType::Vec4, 4);
        s_texColor = createUniform("s_texColor", bgfx::UniformType::Sampler);
        m_texture = createTexture2D(SPRITE_TEXTURE_SIZE, SPRITE_TEXTURE_SIZE, false, 1, bgfx::TextureFormat::BGRA8);

        void* vertices[DebugMesh::Count] = {};
        uint16_t* indices[DebugMesh::Count] = {};
        uint16_t stride = DebugShapeVertex::ms_layout.getStride();

        uint32_t startVertex = 0;
        uint32_t startIndex = 0;

        for (uint32_t mesh = 0; mesh < 4; ++mesh)
        {
            DebugMesh::Enum id = static_cast<DebugMesh::Enum>(DebugMesh::Sphere0 + mesh);

            const uint8_t tess = static_cast<uint8_t>(3 - mesh);
            const uint32_t numVertices = genSphere(tess);
            const uint32_t numIndices = numVertices;

            vertices[id] = BX_ALLOC(m_allocator, numVertices*stride);
            bx::memSet(vertices[id], 0, numVertices * stride);
            genSphere(tess, vertices[id], stride);

            uint16_t* trilist = static_cast<uint16_t*>(BX_ALLOC(m_allocator, numIndices*sizeof(uint16_t)));
            for (uint32_t ii = 0; ii < numIndices; ++ii)
            {
                trilist[ii] = static_cast<uint16_t>(ii);
            }

            uint32_t numLineListIndices = topologyConvert(
                bgfx::TopologyConvert::TriListToLineList
                , nullptr
                , 0
                , trilist
                , numIndices
                , false
            );
            indices[id] = static_cast<uint16_t*>(BX_ALLOC(m_allocator,
                                                          (numIndices + numLineListIndices)*sizeof(uint16_t)));
            uint16_t* indicesOut = indices[id];
            bx::memCopy(indicesOut, trilist, numIndices * sizeof(uint16_t));

            topologyConvert(
                bgfx::TopologyConvert::TriListToLineList
                , &indicesOut[numIndices]
                , numLineListIndices * sizeof(uint16_t)
                , trilist
                , numIndices
                , false
            );

            m_mesh[id].m_startVertex = startVertex;
            m_mesh[id].m_numVertices = numVertices;
            m_mesh[id].m_startIndex[0] = startIndex;
            m_mesh[id].m_numIndices[0] = numIndices;
            m_mesh[id].m_startIndex[1] = startIndex + numIndices;
            m_mesh[id].m_numIndices[1] = numLineListIndices;

            startVertex += numVertices;
            startIndex += numIndices + numLineListIndices;

            BX_FREE(m_allocator, trilist);
        }

        for (uint32_t mesh = 0; mesh < 4; ++mesh)
        {
            DebugMesh::Enum id = static_cast<DebugMesh::Enum>(DebugMesh::Cone0 + mesh);

            const uint32_t num = getCircleLod(static_cast<uint8_t>(mesh));
            const float step = bx::kPi * 2.0f / num;

            const uint32_t numVertices = num + 1;
            const uint32_t numIndices = num * 6;
            const uint32_t numLineListIndices = num * 4;

            vertices[id] = BX_ALLOC(m_allocator, numVertices*stride);
            indices[id] = static_cast<uint16_t*>(BX_ALLOC(m_allocator,
                                                          (numIndices + numLineListIndices)*sizeof(uint16_t)));
            bx::memSet(indices[id], 0, (numIndices + numLineListIndices) * sizeof(uint16_t));

            DebugShapeVertex* vertex = static_cast<DebugShapeVertex*>(vertices[id]);
            uint16_t* index = indices[id];

            vertex[num].m_x = 0.0f;
            vertex[num].m_y = 0.0f;
            vertex[num].m_z = 0.0f;
            vertex[num].m_indices[0] = 1;

            for (uint32_t ii = 0; ii < num; ++ii)
            {
                const float angle = step * ii;

                float xy[2];
                circle(xy, angle);

                vertex[ii].m_x = xy[1];
                vertex[ii].m_y = 0.0f;
                vertex[ii].m_z = xy[0];
                vertex[ii].m_indices[0] = 0;

                index[ii * 3 + 0] = static_cast<uint16_t>(num);
                index[ii * 3 + 1] = static_cast<uint16_t>((ii + 1) % num);
                index[ii * 3 + 2] = static_cast<uint16_t>(ii);

                index[num * 3 + ii * 3 + 0] = 0;
                index[num * 3 + ii * 3 + 1] = static_cast<uint16_t>(ii);
                index[num * 3 + ii * 3 + 2] = static_cast<uint16_t>((ii + 1) % num);

                index[numIndices + ii * 2 + 0] = static_cast<uint16_t>(ii);
                index[numIndices + ii * 2 + 1] = static_cast<uint16_t>(num);

                index[numIndices + num * 2 + ii * 2 + 0] = static_cast<uint16_t>(ii);
                index[numIndices + num * 2 + ii * 2 + 1] = static_cast<uint16_t>((ii + 1) % num);
            }

            m_mesh[id].m_startVertex = startVertex;
            m_mesh[id].m_numVertices = numVertices;
            m_mesh[id].m_startIndex[0] = startIndex;
            m_mesh[id].m_numIndices[0] = numIndices;
            m_mesh[id].m_startIndex[1] = startIndex + numIndices;
            m_mesh[id].m_numIndices[1] = numLineListIndices;

            startVertex += numVertices;
            startIndex += numIndices + numLineListIndices;
        }

        for (uint32_t mesh = 0; mesh < 4; ++mesh)
        {
            DebugMesh::Enum id = static_cast<DebugMesh::Enum>(DebugMesh::Cylinder0 + mesh);

            const uint32_t num = getCircleLod(static_cast<uint8_t>(mesh));
            const float step = bx::kPi * 2.0f / num;

            const uint32_t numVertices = num * 2;
            const uint32_t numIndices = num * 12;
            const uint32_t numLineListIndices = num * 6;

            vertices[id] = BX_ALLOC(m_allocator, numVertices*stride);
            indices[id] = static_cast<uint16_t*>(BX_ALLOC(m_allocator,
                                                          (numIndices + numLineListIndices)*sizeof(uint16_t)));
            bx::memSet(indices[id], 0, (numIndices + numLineListIndices) * sizeof(uint16_t));

            DebugShapeVertex* vertex = static_cast<DebugShapeVertex*>(vertices[id]);
            uint16_t* index = indices[id];

            for (uint32_t ii = 0; ii < num; ++ii)
            {
                const float angle = step * ii;

                float xy[2];
                circle(xy, angle);

                vertex[ii].m_x = xy[1];
                vertex[ii].m_y = 0.0f;
                vertex[ii].m_z = xy[0];
                vertex[ii].m_indices[0] = 0;

                vertex[ii + num].m_x = xy[1];
                vertex[ii + num].m_y = 0.0f;
                vertex[ii + num].m_z = xy[0];
                vertex[ii + num].m_indices[0] = 1;

                index[ii * 6 + 0] = static_cast<uint16_t>(ii + num);
                index[ii * 6 + 1] = static_cast<uint16_t>((ii + 1) % num);
                index[ii * 6 + 2] = static_cast<uint16_t>(ii);
                index[ii * 6 + 3] = static_cast<uint16_t>(ii + num);
                index[ii * 6 + 4] = static_cast<uint16_t>((ii + 1) % num + num);
                index[ii * 6 + 5] = static_cast<uint16_t>((ii + 1) % num);

                index[num * 6 + ii * 6 + 0] = static_cast<uint16_t>(0);
                index[num * 6 + ii * 6 + 1] = static_cast<uint16_t>(ii);
                index[num * 6 + ii * 6 + 2] = static_cast<uint16_t>((ii + 1) % num);
                index[num * 6 + ii * 6 + 3] = static_cast<uint16_t>(num);
                index[num * 6 + ii * 6 + 4] = static_cast<uint16_t>((ii + 1) % num + num);
                index[num * 6 + ii * 6 + 5] = static_cast<uint16_t>(ii + num);

                index[numIndices + ii * 2 + 0] = static_cast<uint16_t>(ii);
                index[numIndices + ii * 2 + 1] = static_cast<uint16_t>(ii + num);

                index[numIndices + num * 2 + ii * 2 + 0] = static_cast<uint16_t>(ii);
                index[numIndices + num * 2 + ii * 2 + 1] = static_cast<uint16_t>((ii + 1) % num);

                index[numIndices + num * 4 + ii * 2 + 0] = static_cast<uint16_t>(num + ii);
                index[numIndices + num * 4 + ii * 2 + 1] = static_cast<uint16_t>(num + (ii + 1) % num);
            }

            m_mesh[id].m_startVertex = startVertex;
            m_mesh[id].m_numVertices = numVertices;
            m_mesh[id].m_startIndex[0] = startIndex;
            m_mesh[id].m_numIndices[0] = numIndices;
            m_mesh[id].m_startIndex[1] = startIndex + numIndices;
            m_mesh[id].m_numIndices[1] = numLineListIndices;

            startVertex += numVertices;
            startIndex += numIndices + numLineListIndices;
        }

        for (uint32_t mesh = 0; mesh < 4; ++mesh)
        {
            DebugMesh::Enum id = static_cast<DebugMesh::Enum>(DebugMesh::Capsule0 + mesh);

            const uint32_t num = getCircleLod(static_cast<uint8_t>(mesh));
            const float step = bx::kPi * 2.0f / num;

            const uint32_t numVertices = num * 2;
            const uint32_t numIndices = num * 6;
            const uint32_t numLineListIndices = num * 6;

            vertices[id] = BX_ALLOC(m_allocator, numVertices*stride);
            indices[id] = static_cast<uint16_t*>(BX_ALLOC(m_allocator,
                                                          (numIndices + numLineListIndices)*sizeof(uint16_t)));
            bx::memSet(indices[id], 0, (numIndices + numLineListIndices) * sizeof(uint16_t));

            DebugShapeVertex* vertex = static_cast<DebugShapeVertex*>(vertices[id]);
            uint16_t* index = indices[id];

            for (uint32_t ii = 0; ii < num; ++ii)
            {
                const float angle = step * ii;

                float xy[2];
                circle(xy, angle);

                vertex[ii].m_x = xy[1];
                vertex[ii].m_y = 0.0f;
                vertex[ii].m_z = xy[0];
                vertex[ii].m_indices[0] = 0;

                vertex[ii + num].m_x = xy[1];
                vertex[ii + num].m_y = 0.0f;
                vertex[ii + num].m_z = xy[0];
                vertex[ii + num].m_indices[0] = 1;

                index[ii * 6 + 0] = static_cast<uint16_t>(ii + num);
                index[ii * 6 + 1] = static_cast<uint16_t>((ii + 1) % num);
                index[ii * 6 + 2] = static_cast<uint16_t>(ii);
                index[ii * 6 + 3] = static_cast<uint16_t>(ii + num);
                index[ii * 6 + 4] = static_cast<uint16_t>((ii + 1) % num + num);
                index[ii * 6 + 5] = static_cast<uint16_t>((ii + 1) % num);

                //				index[num*6+ii*6+0] = uint16_t(0);
                //				index[num*6+ii*6+1] = uint16_t(ii);
                //				index[num*6+ii*6+2] = uint16_t( (ii+1)%num);
                //				index[num*6+ii*6+3] = uint16_t(num);
                //				index[num*6+ii*6+4] = uint16_t( (ii+1)%num+num);
                //				index[num*6+ii*6+5] = uint16_t(ii+num);

                index[numIndices + ii * 2 + 0] = static_cast<uint16_t>(ii);
                index[numIndices + ii * 2 + 1] = static_cast<uint16_t>(ii + num);

                index[numIndices + num * 2 + ii * 2 + 0] = static_cast<uint16_t>(ii);
                index[numIndices + num * 2 + ii * 2 + 1] = static_cast<uint16_t>((ii + 1) % num);

                index[numIndices + num * 4 + ii * 2 + 0] = static_cast<uint16_t>(num + ii);
                index[numIndices + num * 4 + ii * 2 + 1] = static_cast<uint16_t>(num + (ii + 1) % num);
            }

            m_mesh[id].m_startVertex = startVertex;
            m_mesh[id].m_numVertices = numVertices;
            m_mesh[id].m_startIndex[0] = startIndex;
            m_mesh[id].m_numIndices[0] = numIndices;
            m_mesh[id].m_startIndex[1] = startIndex + numIndices;
            m_mesh[id].m_numIndices[1] = numLineListIndices;

            startVertex += numVertices;
            startIndex += numIndices + numLineListIndices;
        }

        m_mesh[DebugMesh::Quad].m_startVertex = startVertex;
        m_mesh[DebugMesh::Quad].m_numVertices = BX_COUNTOF(s_quadVertices);
        m_mesh[DebugMesh::Quad].m_startIndex[0] = startIndex;
        m_mesh[DebugMesh::Quad].m_numIndices[0] = BX_COUNTOF(s_quadIndices);
        m_mesh[DebugMesh::Quad].m_startIndex[1] = 0;
        m_mesh[DebugMesh::Quad].m_numIndices[1] = 0;
        startVertex += BX_COUNTOF(s_quadVertices);
        startIndex += BX_COUNTOF(s_quadIndices);

        m_mesh[DebugMesh::Cube].m_startVertex = startVertex;
        m_mesh[DebugMesh::Cube].m_numVertices = BX_COUNTOF(s_cubeVertices);
        m_mesh[DebugMesh::Cube].m_startIndex[0] = startIndex;
        m_mesh[DebugMesh::Cube].m_numIndices[0] = BX_COUNTOF(s_cubeIndices);
        m_mesh[DebugMesh::Cube].m_startIndex[1] = 0;
        m_mesh[DebugMesh::Cube].m_numIndices[1] = 0;
        startVertex += m_mesh[DebugMesh::Cube].m_numVertices;
        startIndex += m_mesh[DebugMesh::Cube].m_numIndices[0];

        const bgfx::Memory* vb = bgfx::alloc(startVertex * stride);
        const bgfx::Memory* ib = bgfx::alloc(startIndex * sizeof(uint16_t));

        for (uint32_t mesh = DebugMesh::Sphere0; mesh < DebugMesh::Quad; ++mesh)
        {
            DebugMesh::Enum id = static_cast<DebugMesh::Enum>(mesh);
            bx::memCopy(&vb->data[m_mesh[id].m_startVertex * stride]
                        , vertices[id]
                        , m_mesh[id].m_numVertices * stride
            );

            bx::memCopy(&ib->data[m_mesh[id].m_startIndex[0] * sizeof(uint16_t)]
                        , indices[id]
                        , (m_mesh[id].m_numIndices[0] + m_mesh[id].m_numIndices[1]) * sizeof(uint16_t)
            );

            BX_FREE(m_allocator, vertices[id]);
            BX_FREE(m_allocator, indices[id]);
        }

        bx::memCopy(&vb->data[m_mesh[DebugMesh::Quad].m_startVertex * stride]
                    , s_quadVertices
                    , sizeof(s_quadVertices)
        );

        bx::memCopy(&ib->data[m_mesh[DebugMesh::Quad].m_startIndex[0] * sizeof(uint16_t)]
                    , s_quadIndices
                    , sizeof(s_quadIndices)
        );

        bx::memCopy(&vb->data[m_mesh[DebugMesh::Cube].m_startVertex * stride]
                    , s_cubeVertices
                    , sizeof(s_cubeVertices)
        );

        bx::memCopy(&ib->data[m_mesh[DebugMesh::Cube].m_startIndex[0] * sizeof(uint16_t)]
                    , s_cubeIndices
                    , sizeof(s_cubeIndices)
        );

        m_vbh = createVertexBuffer(vb, DebugShapeVertex::ms_layout);
        m_ibh = createIndexBuffer(ib);
    }

    void shutdown()
    {
        bgfx::destroy(m_ibh);
        bgfx::destroy(m_vbh);
        for (uint32_t ii = 0; ii < Program::Count; ++ii)
        {
            bgfx::destroy(m_program[ii]);
        }
        bgfx::destroy(u_params);
        bgfx::destroy(s_texColor);
        bgfx::destroy(m_texture);
    }

    atlas::render::debug::SpriteHandle createSprite(uint16_t _width, uint16_t _height, const void* _data)
    {
        atlas::render::debug::SpriteHandle handle = m_sprite.create(_width, _height);

        if (isValid(handle))
        {
            const Pack2D& pack = m_sprite.get(handle);
            updateTexture2D(
                m_texture
                , 0
                , 0
                , pack.m_X
                , pack.m_Y
                , pack.m_Width
                , pack.m_Height
                , bgfx::copy(_data, pack.m_Width * pack.m_Height * 4)
            );
        }

        return handle;
    }

    void destroy(atlas::render::debug::SpriteHandle _handle)
    {
        m_sprite.destroy(_handle);
    }

    atlas::render::debug::GeometryHandle createGeometry(uint32_t _numVertices,
                                                        const atlas::render::debug::DdVertex* _vertices,
                                                        uint32_t _numIndices, const void* _indices, bool _index32)
    {
        return m_geometry.create(_numVertices, _vertices, _numIndices, _indices, _index32);
    }

    void destroy(atlas::render::debug::GeometryHandle _handle)
    {
        m_geometry.destroy(_handle);
    }

    bx::AllocatorI* m_allocator;

    Sprite m_sprite;
    Geometry m_geometry;

    DebugMesh m_mesh[DebugMesh::Count];

    bgfx::UniformHandle s_texColor;
    bgfx::TextureHandle m_texture;
    bgfx::ProgramHandle m_program[Program::Count];
    bgfx::UniformHandle u_params;

    bgfx::VertexBufferHandle m_vbh;
    bgfx::IndexBufferHandle m_ibh;
};

static DebugDrawShared s_dds;

struct DebugDrawEncoderImpl
{
    DebugDrawEncoderImpl()
        : m_DepthTest(DepthTest::Less)
          , m_State(State::Count)
          , m_DefaultEncoder(nullptr)
    {
    }

    void init(bgfx::Encoder* _encoder)
    {
        m_DefaultEncoder = _encoder;
        m_State = State::Count;
    }

    void shutdown()
    {
    }

    void begin(bgfx::ViewId _viewId, bool _depthTestLess, bgfx::Encoder* _encoder)
    {
        BX_ASSERT(State::Count == m_State, "");

        m_ViewId = _viewId;
        m_Encoder = _encoder == nullptr ? m_DefaultEncoder : _encoder;
        m_State = State::None;
        m_Stack = 0;
        m_DepthTest = _depthTestLess ? DepthTest::Less : DepthTest::Greater;

        m_Pos = 0;
        m_IndexPos = 0;
        m_VertexPos = 0;
        m_PosQuad = 0;

        Attrib& attrib = m_Attrib[0];
        attrib.m_state = 0
            | BGFX_STATE_WRITE_RGB
            | getDepthTestStateValue()
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_WRITE_Z;

        attrib.m_scale = 1.0f;
        attrib.m_spin = 0.0f;
        attrib.m_offset = 0.0f;
        attrib.m_abgr = UINT32_MAX;
        attrib.m_stipple = false;
        attrib.m_wireframe = false;
        attrib.m_lod = 0;

        m_MtxStackCurrent = 0;
        m_MtxStack[m_MtxStackCurrent].Reset();
    }

    void end()
    {
        BX_ASSERT(0 == m_Stack, "Invalid stack %d.", m_Stack);

        flushQuad();
        flush();

        m_Encoder = nullptr;
        m_State = State::Count;
    }

    void push()
    {
        BX_ASSERT(State::Count != m_State, "");
        ++m_Stack;
        m_Attrib[m_Stack] = m_Attrib[m_Stack - 1];
    }

    void pop()
    {
        BX_ASSERT(State::Count != m_State, "");
        const Attrib& curr = m_Attrib[m_Stack];
        const Attrib& prev = m_Attrib[m_Stack - 1];
        if (curr.m_stipple != prev.m_stipple
            || curr.m_state != prev.m_state)
        {
            flush();
        }
        --m_Stack;
    }

    uint64_t getDepthTestStateValue()
    {
        switch(m_DepthTest)
        {
        case DepthTest::Less:
            return BGFX_STATE_DEPTH_TEST_LESS;
        case DepthTest::Greater:
            return BGFX_STATE_DEPTH_TEST_GREATER;
        case DepthTest::Always:
            return BGFX_STATE_DEPTH_TEST_ALWAYS;
        default: BX_ASSERT(false, "Invalid case"); break;
        }
        return 0;
    }

    void setDepthTestLess(bool _depthTestLess)
    {
        BX_ASSERT(State::Count != m_State, "");
        m_DepthTest = _depthTestLess ? DepthTest::Less : DepthTest::Greater;

        Attrib& attrib = m_Attrib[m_Stack];
        if (attrib.m_state & BGFX_STATE_DEPTH_TEST_MASK)
        {
            flush();
            attrib.m_state &= ~BGFX_STATE_DEPTH_TEST_MASK;
            attrib.m_state |= getDepthTestStateValue();
        }
    }

    void setDepthTestAlways()
    {
        BX_ASSERT(State::Count != m_State, "");
        m_DepthTest = DepthTest::Always;
        Attrib& attrib = m_Attrib[m_Stack];
        if (attrib.m_state & BGFX_STATE_DEPTH_TEST_MASK)
        {
            flush();
            attrib.m_state &= ~BGFX_STATE_DEPTH_TEST_MASK;
            attrib.m_state |= getDepthTestStateValue();
        }
    }

    void setTransform(const void* _mtx, uint16_t _num = 1, bool _flush = true)
    {
        BX_ASSERT(State::Count != m_State, "");
        if (_flush)
        {
            flush();
        }

        MatrixStack& stack = m_MtxStack[m_MtxStackCurrent];

        if (nullptr == _mtx)
        {
            stack.Reset();
            return;
        }

        bgfx::Transform transform;
        stack.m_Mtx = m_Encoder->allocTransform(&transform, _num);
        stack.m_Num = _num;
        stack.m_Data = transform.data;
        bx::memCopy(transform.data, _mtx, _num * 64);
    }

    void setTranslate(float _x, float _y, float _z)
    {
        float mtx[16];
        bx::mtxTranslate(mtx, _x, _y, _z);
        setTransform(mtx);
    }

    void setTranslate(const float* _pos)
    {
        setTranslate(_pos[0], _pos[1], _pos[2]);
    }

    void pushTransform(const void* _mtx, uint16_t _num, bool _flush = true)
    {
        BX_ASSERT(m_MtxStackCurrent < BX_COUNTOF(m_MtxStack), "Out of matrix stack!");
        BX_ASSERT(State::Count != m_State, "");
        if (_flush)
        {
            flush();
        }

        float* mtx = nullptr;

        const MatrixStack& stack = m_MtxStack[m_MtxStackCurrent];

        if (nullptr == stack.m_Data)
        {
            mtx = (float*)_mtx;
        }
        else
        {
            mtx = static_cast<float*>(alloca(_num * 64));
            for (uint16_t ii = 0; ii < _num; ++ii)
            {
                const float* mtxTransform = static_cast<const float*>(_mtx);
                bx::mtxMul(&mtx[ii * 16], &mtxTransform[ii * 16], stack.m_Data);
            }
        }

        m_MtxStackCurrent++;
        setTransform(mtx, _num, _flush);
    }

    void popTransform(bool _flush = true)
    {
        BX_ASSERT(State::Count != m_State, "");
        if (_flush)
        {
            flush();
        }

        m_MtxStackCurrent--;
    }

    void pushTranslate(float _x, float _y, float _z)
    {
        float mtx[16];
        bx::mtxTranslate(mtx, _x, _y, _z);
        pushTransform(mtx, 1);
    }

    void pushTranslate(const bx::Vec3& _pos)
    {
        pushTranslate(_pos.x, _pos.y, _pos.z);
    }

    void setState(bool _depthTest, bool _depthWrite, bool _clockwise)
    {
        const uint64_t depthTest = getDepthTestStateValue();

        uint64_t state = m_Attrib[m_Stack].m_state & ~(0
            | BGFX_STATE_DEPTH_TEST_MASK
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_CULL_CCW
        );

        state |= _depthTest
                     ? depthTest
                     : 0;

        state |= _depthWrite
                     ? BGFX_STATE_WRITE_Z
                     : 0;

        state |= _clockwise
                     ? BGFX_STATE_CULL_CW
                     : BGFX_STATE_CULL_CCW;

        if (m_Attrib[m_Stack].m_state != state)
        {
            flush();
        }

        m_Attrib[m_Stack].m_state = state;
    }

    void setColor(uint32_t _abgr)
    {
        BX_ASSERT(State::Count != m_State, "");
        m_Attrib[m_Stack].m_abgr = _abgr;
    }

    void setLod(uint8_t _lod)
    {
        BX_ASSERT(State::Count != m_State, "");
        m_Attrib[m_Stack].m_lod = _lod;
    }

    void setWireframe(bool _wireframe)
    {
        BX_ASSERT(State::Count != m_State, "");
        m_Attrib[m_Stack].m_wireframe = _wireframe;
    }

    void setStipple(bool _stipple, float _scale = 1.0f, float _offset = 0.0f)
    {
        BX_ASSERT(State::Count != m_State, "");

        Attrib& attrib = m_Attrib[m_Stack];

        if (attrib.m_stipple != _stipple)
        {
            flush();
        }

        attrib.m_stipple = _stipple;
        attrib.m_offset = _offset;
        attrib.m_scale = _scale;
    }

    void setSpin(float _spin)
    {
        Attrib& attrib = m_Attrib[m_Stack];
        attrib.m_spin = _spin;
    }

    void moveTo(float _x, float _y, float _z = 0.0f)
    {
        BX_ASSERT(State::Count != m_State, "");

        softFlush();

        m_State = State::MoveTo;

        DebugVertex& vertex = m_Cache[m_Pos];
        vertex.m_X = _x;
        vertex.m_Y = _y;
        vertex.m_Z = _z;

        Attrib& attrib = m_Attrib[m_Stack];
        vertex.m_Abgr = attrib.m_abgr;
        vertex.m_Len = attrib.m_offset;

        m_VertexPos = m_Pos;
    }

    void moveTo(const bx::Vec3& _pos)
    {
        BX_ASSERT(State::Count != m_State, "");
        moveTo(_pos.x, _pos.y, _pos.z);
    }

    void moveTo(atlas::render::debug::Axis::Enum _axis, float _x, float _y)
    {
        moveTo(getPoint(_axis, _x, _y));
    }

    void lineTo(float _x, float _y, float _z = 0.0f)
    {
        BX_ASSERT(State::Count != m_State, "");
        if (State::None == m_State)
        {
            moveTo(_x, _y, _z);
            return;
        }

        if (m_Pos + 2 > static_cast<uint16_t>(BX_COUNTOF(m_Cache)))
        {
            uint32_t pos = m_Pos;
            uint32_t vertexPos = m_VertexPos;

            flush();

            bx::memCopy(&m_Cache[0], &m_Cache[vertexPos], sizeof(DebugVertex));
            if (vertexPos == pos)
            {
                m_Pos = 1;
            }
            else
            {
                bx::memCopy(&m_Cache[1], &m_Cache[pos - 1], sizeof(DebugVertex));
                m_Pos = 2;
            }

            m_State = State::LineTo;
        }
        else if (State::MoveTo == m_State)
        {
            ++m_Pos;
            m_State = State::LineTo;
        }

        uint16_t prev = m_Pos - 1;
        uint16_t curr = m_Pos++;
        DebugVertex& vertex = m_Cache[curr];
        vertex.m_X = _x;
        vertex.m_Y = _y;
        vertex.m_Z = _z;

        Attrib& attrib = m_Attrib[m_Stack];
        vertex.m_Abgr = attrib.m_abgr;
        vertex.m_Len = attrib.m_offset;

        float len = length(sub(bx::load<bx::Vec3>(&vertex.m_X), bx::load<bx::Vec3>(&m_Cache[prev].m_X))) * attrib.
            m_scale;
        vertex.m_Len = m_Cache[prev].m_Len + len;

        m_Indices[m_IndexPos++] = prev;
        m_Indices[m_IndexPos++] = curr;
    }

    void lineTo(const bx::Vec3& _pos)
    {
        BX_ASSERT(State::Count != m_State, "");
        lineTo(_pos.x, _pos.y, _pos.z);
    }

    void lineTo(atlas::render::debug::Axis::Enum _axis, float _x, float _y)
    {
        lineTo(getPoint(_axis, _x, _y));
    }

    void close()
    {
        BX_ASSERT(State::Count != m_State, "");
        DebugVertex& vertex = m_Cache[m_VertexPos];
        lineTo(vertex.m_X, vertex.m_Y, vertex.m_Z);

        m_State = State::None;
    }

    void draw(const bx::Aabb& _aabb)
    {
        const Attrib& attrib = m_Attrib[m_Stack];
        if (attrib.m_wireframe)
        {
            moveTo(_aabb.min.x, _aabb.min.y, _aabb.min.z);
            lineTo(_aabb.max.x, _aabb.min.y, _aabb.min.z);
            lineTo(_aabb.max.x, _aabb.max.y, _aabb.min.z);
            lineTo(_aabb.min.x, _aabb.max.y, _aabb.min.z);
            close();

            moveTo(_aabb.min.x, _aabb.min.y, _aabb.max.z);
            lineTo(_aabb.max.x, _aabb.min.y, _aabb.max.z);
            lineTo(_aabb.max.x, _aabb.max.y, _aabb.max.z);
            lineTo(_aabb.min.x, _aabb.max.y, _aabb.max.z);
            close();

            moveTo(_aabb.min.x, _aabb.min.y, _aabb.min.z);
            lineTo(_aabb.min.x, _aabb.min.y, _aabb.max.z);

            moveTo(_aabb.max.x, _aabb.min.y, _aabb.min.z);
            lineTo(_aabb.max.x, _aabb.min.y, _aabb.max.z);

            moveTo(_aabb.min.x, _aabb.max.y, _aabb.min.z);
            lineTo(_aabb.min.x, _aabb.max.y, _aabb.max.z);

            moveTo(_aabb.max.x, _aabb.max.y, _aabb.min.z);
            lineTo(_aabb.max.x, _aabb.max.y, _aabb.max.z);
        }
        else
        {
            bx::Obb obb;
            toObb(obb, _aabb);
            draw(DebugMesh::Cube, obb.mtx, 1, false);
        }
    }

    void draw(const bx::Cylinder& _cylinder, bool _capsule)
    {
        drawCylinder(_cylinder.pos, _cylinder.end, _cylinder.radius, _capsule);
    }

    void draw(const bx::Disk& _disk)
    {
        drawCircle(_disk.normal, _disk.center, _disk.radius, 0.0f);
    }

    void draw(const bx::Obb& _obb)
    {
        const Attrib& attrib = m_Attrib[m_Stack];
        if (attrib.m_wireframe)
        {
            pushTransform(_obb.mtx, 1);

            moveTo(-1.0f, -1.0f, -1.0f);
            lineTo(1.0f, -1.0f, -1.0f);
            lineTo(1.0f, 1.0f, -1.0f);
            lineTo(-1.0f, 1.0f, -1.0f);
            close();

            moveTo(-1.0f, 1.0f, 1.0f);
            lineTo(1.0f, 1.0f, 1.0f);
            lineTo(1.0f, -1.0f, 1.0f);
            lineTo(-1.0f, -1.0f, 1.0f);
            close();

            moveTo(1.0f, -1.0f, -1.0f);
            lineTo(1.0f, -1.0f, 1.0f);

            moveTo(1.0f, 1.0f, -1.0f);
            lineTo(1.0f, 1.0f, 1.0f);

            moveTo(-1.0f, 1.0f, -1.0f);
            lineTo(-1.0f, 1.0f, 1.0f);

            moveTo(-1.0f, -1.0f, -1.0f);
            lineTo(-1.0f, -1.0f, 1.0f);

            popTransform();
        }
        else
        {
            draw(DebugMesh::Cube, _obb.mtx, 1, false);
        }
    }

    void draw(const bx::Sphere& _sphere)
    {
        const Attrib& attrib = m_Attrib[m_Stack];
        float mtx[16];
        bx::mtxSRT(mtx
                   , _sphere.radius
                   , _sphere.radius
                   , _sphere.radius
                   , 0.0f
                   , 0.0f
                   , 0.0f
                   , _sphere.center.x
                   , _sphere.center.y
                   , _sphere.center.z
        );
        uint8_t lod = attrib.m_lod > DebugMesh::SphereMaxLod
                          ? static_cast<uint8_t>(DebugMesh::SphereMaxLod)
                          : attrib.m_lod;
        draw(static_cast<DebugMesh::Enum>(DebugMesh::Sphere0 + lod), mtx, 1, attrib.m_wireframe);
    }

    void draw(const bx::Triangle& _triangle)
    {
        Attrib& attrib = m_Attrib[m_Stack];
        if (attrib.m_wireframe)
        {
            moveTo(_triangle.v0);
            lineTo(_triangle.v1);
            lineTo(_triangle.v2);
            close();
        }
        else
        {
            BX_STATIC_ASSERT(sizeof(atlas::render::debug::DdVertex) == sizeof(bx::Vec3), "");

            uint64_t old = attrib.m_state;
            attrib.m_state &= ~BGFX_STATE_CULL_MASK;

            draw(false, 3, reinterpret_cast<const atlas::render::debug::DdVertex*>(&_triangle.v0.x), 0, nullptr);

            attrib.m_state = old;
        }
    }

    void setUParams(const Attrib& _attrib, bool _wireframe)
    {
        const float flip = 0 == (_attrib.m_state & BGFX_STATE_CULL_CCW) ? 1.0f : -1.0f;
        const uint8_t alpha = _attrib.m_abgr >> 24;

        float params[4][4] =
        {
            {
                // lightDir
                0.0f * flip,
                -1.0f * flip,
                0.0f * flip,
                3.0f, // shininess
            },
            {
                // skyColor
                1.0f,
                0.9f,
                0.8f,
                0.0f, // unused
            },
            {
                // groundColor.xyz0
                0.2f,
                0.22f,
                0.5f,
                0.0f, // unused
            },
            {
                // matColor
                ((_attrib.m_abgr) & 0xff) / 255.0f,
                ((_attrib.m_abgr >> 8) & 0xff) / 255.0f,
                ((_attrib.m_abgr >> 16) & 0xff) / 255.0f,
                (alpha) / 255.0f,
            },
        };

        store(params[0], normalize(bx::load<bx::Vec3>(params[0])));
        m_Encoder->setUniform(s_dds.u_params, params, 4);

        m_Encoder->setState(0
            | _attrib.m_state
            | (_wireframe
                   ? BGFX_STATE_PT_LINES | BGFX_STATE_LINEAA | BGFX_STATE_BLEND_ALPHA
                   : (alpha < 0xff)
                   ? BGFX_STATE_BLEND_ALPHA
                   : 0)
        );
    }

    void draw(atlas::render::debug::GeometryHandle _handle)
    {
        const Geometry::Geometry& geometry = s_dds.m_geometry.m_geometry[_handle.m_Idx];
        m_Encoder->setVertexBuffer(0, geometry.m_vbh);

        const Attrib& attrib = m_Attrib[m_Stack];
        const bool wireframe = attrib.m_wireframe;
        setUParams(attrib, wireframe);

        if (wireframe)
        {
            m_Encoder->setIndexBuffer(
                geometry.m_ibh
                , geometry.m_topologyNumIndices[0]
                , geometry.m_topologyNumIndices[1]
            );
        }
        else if (0 != geometry.m_topologyNumIndices[0])
        {
            m_Encoder->setIndexBuffer(
                geometry.m_ibh
                , 0
                , geometry.m_topologyNumIndices[0]
            );
        }

        m_Encoder->setTransform(m_MtxStack[m_MtxStackCurrent].m_Mtx);
        bgfx::ProgramHandle program = s_dds.m_program[wireframe ? Program::FillMesh : Program::FillLitMesh];
        m_Encoder->submit(m_ViewId, program);
    }

    void draw(bool _lineList, uint32_t _numVertices, const atlas::render::debug::DdVertex* _vertices,
              uint32_t _numIndices, const uint16_t* _indices)
    {
        flush();

        if (_numVertices == getAvailTransientVertexBuffer(_numVertices, DebugMeshVertex::ms_layout))
        {
            bgfx::TransientVertexBuffer tvb;
            allocTransientVertexBuffer(&tvb, _numVertices, DebugMeshVertex::ms_layout);
            bx::memCopy(tvb.data, _vertices, _numVertices * DebugMeshVertex::ms_layout.m_stride);
            m_Encoder->setVertexBuffer(0, &tvb);

            const Attrib& attrib = m_Attrib[m_Stack];
            const bool wireframe = _lineList || attrib.m_wireframe;
            setUParams(attrib, wireframe);

            if (0 < _numIndices)
            {
                uint32_t numIndices = _numIndices;
                bgfx::TransientIndexBuffer tib;
                if (!_lineList && wireframe)
                {
                    numIndices = topologyConvert(
                        bgfx::TopologyConvert::TriListToLineList
                        , nullptr
                        , 0
                        , _indices
                        , _numIndices
                        , false
                    );

                    allocTransientIndexBuffer(&tib, numIndices);
                    topologyConvert(
                        bgfx::TopologyConvert::TriListToLineList
                        , tib.data
                        , numIndices * sizeof(uint16_t)
                        , _indices
                        , _numIndices
                        , false
                    );
                }
                else
                {
                    allocTransientIndexBuffer(&tib, numIndices);
                    bx::memCopy(tib.data, _indices, numIndices * sizeof(uint16_t));
                }

                m_Encoder->setIndexBuffer(&tib);
            }

            m_Encoder->setTransform(m_MtxStack[m_MtxStackCurrent].m_Mtx);
            bgfx::ProgramHandle program = s_dds.m_program[wireframe
                                                              ? Program::FillMesh
                                                              : Program::FillLitMesh
            ];
            m_Encoder->submit(m_ViewId, program);
        }
    }

    void drawFrustum(const float* _viewProj)
    {
        bx::Plane planes[6] = {
            bx::init::None, bx::init::None, bx::init::None, bx::init::None, bx::init::None, bx::init::None
        };
        buildFrustumPlanes(planes, _viewProj);

        const bx::Vec3 points[8] =
        {
            intersectPlanes(planes[0], planes[2], planes[4]),
            intersectPlanes(planes[0], planes[3], planes[4]),
            intersectPlanes(planes[0], planes[3], planes[5]),
            intersectPlanes(planes[0], planes[2], planes[5]),
            intersectPlanes(planes[1], planes[2], planes[4]),
            intersectPlanes(planes[1], planes[3], planes[4]),
            intersectPlanes(planes[1], planes[3], planes[5]),
            intersectPlanes(planes[1], planes[2], planes[5]),
        };

        moveTo(points[0]);
        lineTo(points[1]);
        lineTo(points[2]);
        lineTo(points[3]);
        close();

        moveTo(points[4]);
        lineTo(points[5]);
        lineTo(points[6]);
        lineTo(points[7]);
        close();

        moveTo(points[0]);
        lineTo(points[4]);

        moveTo(points[1]);
        lineTo(points[5]);

        moveTo(points[2]);
        lineTo(points[6]);

        moveTo(points[3]);
        lineTo(points[7]);
    }

    void drawFrustum(const void* _viewProj)
    {
        drawFrustum(static_cast<const float*>(_viewProj));
    }

    void drawArc(atlas::render::debug::Axis::Enum _axis, float _x, float _y, float _z, float _radius, float _degrees)
    {
        const Attrib& attrib = m_Attrib[m_Stack];
        const uint32_t num = getCircleLod(attrib.m_lod);
        const float step = bx::kPi * 2.0f / num;

        _degrees = bx::wrap(_degrees, 360.0f);

        bx::Vec3 pos = getPoint(
            _axis
            , bx::sin(step * 0) * _radius
            , bx::cos(step * 0) * _radius
        );

        moveTo({pos.x + _x, pos.y + _y, pos.z + _z});

        uint32_t n = static_cast<uint32_t>(num * _degrees / 360.0f);

        for (uint32_t ii = 1; ii < n + 1; ++ii)
        {
            pos = getPoint(
                _axis
                , bx::sin(step * ii) * _radius
                , bx::cos(step * ii) * _radius
            );
            lineTo({pos.x + _x, pos.y + _y, pos.z + _z});
        }

        moveTo(_x, _y, _z);
        pos = getPoint(
            _axis
            , bx::sin(step * 0) * _radius
            , bx::cos(step * 0) * _radius
        );
        lineTo({pos.x + _x, pos.y + _y, pos.z + _z});

        pos = getPoint(
            _axis
            , bx::sin(step * n) * _radius
            , bx::cos(step * n) * _radius
        );
        moveTo({pos.x + _x, pos.y + _y, pos.z + _z});
        lineTo(_x, _y, _z);
    }

    void drawCircle(const bx::Vec3& _normal, const bx::Vec3& _center, float _radius, float _weight)
    {
        const Attrib& attrib = m_Attrib[m_Stack];
        const uint32_t num = getCircleLod(attrib.m_lod);
        const float step = bx::kPi * 2.0f / num;
        _weight = bx::clamp(_weight, 0.0f, 2.0f);

        bx::Vec3 udir(bx::init::None);
        bx::Vec3 vdir(bx::init::None);
        calcTangentFrame(udir, vdir, _normal, attrib.m_spin);

        float xy0[2];
        float xy1[2];
        circle(xy0, 0.0f);
        Squircle(xy1, 0.0f);

        bx::Vec3 pos = mul(udir, bx::lerp(xy0[0], xy1[0], _weight) * _radius);
        bx::Vec3 tmp0 = mul(vdir, bx::lerp(xy0[1], xy1[1], _weight) * _radius);
        bx::Vec3 tmp1 = add(pos, tmp0);
        bx::Vec3 tmp2 = add(tmp1, _center);
        moveTo(tmp2);

        for (uint32_t ii = 1; ii < num; ++ii)
        {
            float angle = step * ii;
            circle(xy0, angle);
            Squircle(xy1, angle);

            pos = mul(udir, bx::lerp(xy0[0], xy1[0], _weight) * _radius);
            tmp0 = mul(vdir, bx::lerp(xy0[1], xy1[1], _weight) * _radius);
            tmp1 = add(pos, tmp0);
            tmp2 = add(tmp1, _center);
            lineTo(tmp2);
        }

        close();
    }

    void drawCircle(atlas::render::debug::Axis::Enum _axis, float _x, float _y, float _z, float _radius, float _weight)
    {
        const Attrib& attrib = m_Attrib[m_Stack];
        const uint32_t num = getCircleLod(attrib.m_lod);
        const float step = bx::kPi * 2.0f / num;
        _weight = bx::clamp(_weight, 0.0f, 2.0f);

        float xy0[2];
        float xy1[2];
        circle(xy0, 0.0f);
        Squircle(xy1, 0.0f);

        bx::Vec3 pos = getPoint(
            _axis
            , bx::lerp(xy0[0], xy1[0], _weight) * _radius
            , bx::lerp(xy0[1], xy1[1], _weight) * _radius
        );

        moveTo({pos.x + _x, pos.y + _y, pos.z + _z});

        for (uint32_t ii = 1; ii < num; ++ii)
        {
            float angle = step * ii;
            circle(xy0, angle);
            Squircle(xy1, angle);

            pos = getPoint(
                _axis
                , bx::lerp(xy0[0], xy1[0], _weight) * _radius
                , bx::lerp(xy0[1], xy1[1], _weight) * _radius
            );
            lineTo({pos.x + _x, pos.y + _y, pos.z + _z});
        }
        close();
    }

    void drawQuad(const bx::Vec3& _normal, const bx::Vec3& _center, float _size)
    {
        const Attrib& attrib = m_Attrib[m_Stack];
        if (attrib.m_wireframe)
        {
            bx::Vec3 udir(bx::init::None);
            bx::Vec3 vdir(bx::init::None);
            calcTangentFrame(udir, vdir, _normal, attrib.m_spin);

            const float halfExtent = _size * 0.5f;

            const bx::Vec3 umin = mul(udir, -halfExtent);
            const bx::Vec3 umax = mul(udir, halfExtent);
            const bx::Vec3 vmin = mul(vdir, -halfExtent);
            const bx::Vec3 vmax = mul(vdir, halfExtent);
            const bx::Vec3 center = _center;

            moveTo(add(center, add(umin, vmin)));
            lineTo(add(center, add(umax, vmin)));
            lineTo(add(center, add(umax, vmax)));
            lineTo(add(center, add(umin, vmax)));

            close();
        }
        else
        {
            float mtx[16];
            mtxFromNormal(mtx, _normal, _size * 0.5f, _center, attrib.m_spin);
            draw(DebugMesh::Quad, mtx, 1, false);
        }
    }

    void drawQuad(atlas::render::debug::SpriteHandle _handle, const bx::Vec3& _normal, const bx::Vec3& _center,
                  float _size)
    {
        if (!isValid(_handle))
        {
            drawQuad(_normal, _center, _size);
            return;
        }

        if (m_PosQuad == BX_COUNTOF(m_CacheQuad))
        {
            flushQuad();
        }

        const Attrib& attrib = m_Attrib[m_Stack];

        bx::Vec3 udir(bx::init::None);
        bx::Vec3 vdir(bx::init::None);
        calcTangentFrame(udir, vdir, _normal, attrib.m_spin);

        const Pack2D& pack = s_dds.m_sprite.get(_handle);
        const float invTextureSize = 1.0f / SPRITE_TEXTURE_SIZE;
        const float us = pack.m_X * invTextureSize;
        const float vs = pack.m_Y * invTextureSize;
        const float ue = (pack.m_X + pack.m_Width) * invTextureSize;
        const float ve = (pack.m_Y + pack.m_Height) * invTextureSize;

        const float aspectRatio = static_cast<float>(pack.m_Width) / static_cast<float>(pack.m_Height);
        const float halfExtentU = aspectRatio * _size * 0.5f;
        const float halfExtentV = 1.0f / aspectRatio * _size * 0.5f;

        const bx::Vec3 umin = mul(udir, -halfExtentU);
        const bx::Vec3 umax = mul(udir, halfExtentU);
        const bx::Vec3 vmin = mul(vdir, -halfExtentV);
        const bx::Vec3 vmax = mul(vdir, halfExtentV);
        const bx::Vec3 center = _center;

        DebugUvVertex* vertex = &m_CacheQuad[m_PosQuad];
        m_PosQuad += 4;

        store(&vertex->m_x, add(center, add(umin, vmin)));
        vertex->m_u = us;
        vertex->m_v = vs;
        vertex->m_abgr = attrib.m_abgr;
        ++vertex;

        store(&vertex->m_x, add(center, add(umax, vmin)));
        vertex->m_u = ue;
        vertex->m_v = vs;
        vertex->m_abgr = attrib.m_abgr;
        ++vertex;

        store(&vertex->m_x, add(center, add(umin, vmax)));
        vertex->m_u = us;
        vertex->m_v = ve;
        vertex->m_abgr = attrib.m_abgr;
        ++vertex;

        store(&vertex->m_x, add(center, add(umax, vmax)));
        vertex->m_u = ue;
        vertex->m_v = ve;
        vertex->m_abgr = attrib.m_abgr;
        ++vertex;
    }

    void drawQuad(bgfx::TextureHandle _handle, const bx::Vec3& _normal, const bx::Vec3& _center, float _size)
    {
    }

    void drawCone(const bx::Vec3& _from, const bx::Vec3& _to, float _radius)
    {
        const Attrib& attrib = m_Attrib[m_Stack];

        const bx::Vec3 normal = normalize(sub(_from, _to));

        float mtx[2][16];
        mtxFromNormal(mtx[0], normal, _radius, _from, attrib.m_spin);

        bx::memCopy(mtx[1], mtx[0], 64);
        mtx[1][12] = _to.x;
        mtx[1][13] = _to.y;
        mtx[1][14] = _to.z;

        uint8_t lod = attrib.m_lod > DebugMesh::ConeMaxLod
                          ? static_cast<uint8_t>(DebugMesh::ConeMaxLod)
                          : attrib.m_lod;
        draw(static_cast<DebugMesh::Enum>(DebugMesh::Cone0 + lod), mtx[0], 2, attrib.m_wireframe);
    }

    void drawCylinder(const bx::Vec3& _from, const bx::Vec3& _to, float _radius, bool _capsule)
    {
        const Attrib& attrib = m_Attrib[m_Stack];
        const bx::Vec3 normal = normalize(sub(_from, _to));

        float mtx[2][16];
        mtxFromNormal(mtx[0], normal, _radius, _from, attrib.m_spin);

        bx::memCopy(mtx[1], mtx[0], 64);
        mtx[1][12] = _to.x;
        mtx[1][13] = _to.y;
        mtx[1][14] = _to.z;

        if (_capsule)
        {
            uint8_t lod = attrib.m_lod > DebugMesh::CapsuleMaxLod
                              ? static_cast<uint8_t>(DebugMesh::CapsuleMaxLod)
                              : attrib.m_lod;
            draw(static_cast<DebugMesh::Enum>(DebugMesh::Capsule0 + lod), mtx[0], 2, attrib.m_wireframe);

            bx::Sphere sphere;
            sphere.center = _from;
            sphere.radius = _radius;
            draw(sphere);

            sphere.center = _to;
            draw(sphere);
        }
        else
        {
            uint8_t lod = attrib.m_lod > DebugMesh::CylinderMaxLod
                              ? static_cast<uint8_t>(DebugMesh::CylinderMaxLod)
                              : attrib.m_lod;
            draw(static_cast<DebugMesh::Enum>(DebugMesh::Cylinder0 + lod), mtx[0], 2, attrib.m_wireframe);
        }
    }

    void drawAxis(float _x, float _y, float _z, float _len, atlas::render::debug::Axis::Enum _highlight,
                  float xThickness, float yThickness, float zThickness)
    {
        push();

        const bx::Vec3 from = {_x, _y, _z};
        bx::Vec3 mid(bx::init::None);
        bx::Vec3 to(bx::init::None);
        if (xThickness > 0.0f)
        {
            setColor(atlas::render::debug::Axis::X == _highlight ? 0xff00ffff : 0xff0000ff);
            mid = {_x + _len - xThickness, _y, _z};
            to = {_x + _len, _y, _z};
            drawCylinder(from, mid, xThickness, false);
            drawCone(mid, to, xThickness);
        }
        else
        {
            setColor(atlas::render::debug::Axis::X == _highlight ? 0xff00ffff : 0xff0000ff);
            moveTo(_x, _y, _z);
            lineTo(_x + _len, _y, _z);
        }

        if (yThickness > 0.0f)
        {
            setColor(atlas::render::debug::Axis::Y == _highlight ? 0xff00ffff : 0xff00ff00);
            mid = {_x, _y + _len - yThickness, _z};
            to = {_x, _y + _len, _z};
            drawCylinder(from, mid, yThickness, false);
            drawCone(mid, to, yThickness);
        }
        else
        {
            setColor(atlas::render::debug::Axis::Y == _highlight ? 0xff00ffff : 0xff00ff00);
            moveTo(_x, _y, _z);
            lineTo(_x, _y + _len, _z);
        }

        if (zThickness > 0.0f)
        {
            setColor(atlas::render::debug::Axis::Z == _highlight ? 0xff00ffff : 0xffff0000);
            mid = {_x, _y, _z + _len - zThickness};
            to = {_x, _y, _z + _len};
            drawCylinder(from, mid, zThickness, false);
            drawCone(mid, to, zThickness);
        }
        else
        {
            setColor(atlas::render::debug::Axis::Z == _highlight ? 0xff00ffff : 0xffff0000);
            moveTo(_x, _y, _z);
            lineTo(_x, _y, _z + _len);
        }

        pop();
    }

    void drawAxis(float _x, float _y, float _z, float _len, atlas::render::debug::Axis::Enum _highlight,
                  float _thickness)
    {
        push();

        if (_thickness > 0.0f)
        {
            const bx::Vec3 from = {_x, _y, _z};
            bx::Vec3 mid(bx::init::None);
            bx::Vec3 to(bx::init::None);

            setColor(atlas::render::debug::Axis::X == _highlight ? 0xff00ffff : 0xff0000ff);
            mid = {_x + _len - _thickness, _y, _z};
            to = {_x + _len, _y, _z};
            drawCylinder(from, mid, _thickness, false);
            drawCone(mid, to, _thickness);

            setColor(atlas::render::debug::Axis::Y == _highlight ? 0xff00ffff : 0xff00ff00);
            mid = {_x, _y + _len - _thickness, _z};
            to = {_x, _y + _len, _z};
            drawCylinder(from, mid, _thickness, false);
            drawCone(mid, to, _thickness);

            setColor(atlas::render::debug::Axis::Z == _highlight ? 0xff00ffff : 0xffff0000);
            mid = {_x, _y, _z + _len - _thickness};
            to = {_x, _y, _z + _len};
            drawCylinder(from, mid, _thickness, false);
            drawCone(mid, to, _thickness);
        }
        else
        {
            setColor(atlas::render::debug::Axis::X == _highlight ? 0xff00ffff : 0xff0000ff);
            moveTo(_x, _y, _z);
            lineTo(_x + _len, _y, _z);

            setColor(atlas::render::debug::Axis::Y == _highlight ? 0xff00ffff : 0xff00ff00);
            moveTo(_x, _y, _z);
            lineTo(_x, _y + _len, _z);

            setColor(atlas::render::debug::Axis::Z == _highlight ? 0xff00ffff : 0xffff0000);
            moveTo(_x, _y, _z);
            lineTo(_x, _y, _z + _len);
        }

        pop();
    }

    void drawGrid(const bx::Vec3& _normal, const bx::Vec3& _center, uint32_t _size, float _step)
    {
        const Attrib& attrib = m_Attrib[m_Stack];

        bx::Vec3 udir(bx::init::None);
        bx::Vec3 vdir(bx::init::None);
        calcTangentFrame(udir, vdir, _normal, attrib.m_spin);

        udir = mul(udir, _step);
        vdir = mul(vdir, _step);

        const uint32_t num = (_size / 2) * 2 + 1;
        const float halfExtent = static_cast<float>(_size / 2);

        const bx::Vec3 umin = mul(udir, -halfExtent);
        const bx::Vec3 umax = mul(udir, halfExtent);
        const bx::Vec3 vmin = mul(vdir, -halfExtent);
        const bx::Vec3 vmax = mul(vdir, halfExtent);

        bx::Vec3 xs = add(_center, add(umin, vmin));
        bx::Vec3 xe = add(_center, add(umax, vmin));
        bx::Vec3 ys = add(_center, add(umin, vmin));
        bx::Vec3 ye = add(_center, add(umin, vmax));

        for (uint32_t ii = 0; ii < num; ++ii)
        {
            moveTo(xs);
            lineTo(xe);
            xs = add(xs, vdir);
            xe = add(xe, vdir);

            moveTo(ys);
            lineTo(ye);
            ys = add(ys, udir);
            ye = add(ye, udir);
        }
    }

    void drawGrid(atlas::render::debug::Axis::Enum _axis, const bx::Vec3& _center, uint32_t _size, float _step)
    {
        push();
        pushTranslate(_center);

        const uint32_t num = (_size / 2) * 2 - 1;
        const float halfExtent = static_cast<float>(_size / 2) * _step;

        setColor(0xff606060);
        float yy = -halfExtent + _step;
        for (uint32_t ii = 0; ii < num; ++ii)
        {
            moveTo(_axis, -halfExtent, yy);
            lineTo(_axis, halfExtent, yy);

            moveTo(_axis, yy, -halfExtent);
            lineTo(_axis, yy, halfExtent);

            yy += _step;
        }

        setColor(0xff101010);
        moveTo(_axis, -halfExtent, -halfExtent);
        lineTo(_axis, -halfExtent, halfExtent);
        lineTo(_axis, halfExtent, halfExtent);
        lineTo(_axis, halfExtent, -halfExtent);
        close();

        moveTo(_axis, -halfExtent, 0.0f);
        lineTo(_axis, halfExtent, 0.0f);

        moveTo(_axis, 0.0f, -halfExtent);
        lineTo(_axis, 0.0f, halfExtent);

        popTransform();
        pop();
    }

    void drawOrb(float _x, float _y, float _z, float _radius, atlas::render::debug::Axis::Enum _hightlight)
    {
        push();

        setColor(atlas::render::debug::Axis::X == _hightlight ? 0xff00ffff : 0xff0000ff);
        drawCircle(atlas::render::debug::Axis::X, _x, _y, _z, _radius, 0.0f);

        setColor(atlas::render::debug::Axis::Y == _hightlight ? 0xff00ffff : 0xff00ff00);
        drawCircle(atlas::render::debug::Axis::Y, _x, _y, _z, _radius, 0.0f);

        setColor(atlas::render::debug::Axis::Z == _hightlight ? 0xff00ffff : 0xffff0000);
        drawCircle(atlas::render::debug::Axis::Z, _x, _y, _z, _radius, 0.0f);

        pop();
    }

    void draw(DebugMesh::Enum _mesh, const float* _mtx, uint16_t _num, bool _wireframe)
    {
        pushTransform(_mtx, _num, false /* flush */);

        const DebugMesh& mesh = s_dds.m_mesh[_mesh];

        if (0 != mesh.m_numIndices[_wireframe])
        {
            m_Encoder->setIndexBuffer(s_dds.m_ibh
                                      , mesh.m_startIndex[_wireframe]
                                      , mesh.m_numIndices[_wireframe]
            );
        }

        const Attrib& attrib = m_Attrib[m_Stack];
        setUParams(attrib, _wireframe);

        MatrixStack& stack = m_MtxStack[m_MtxStackCurrent];
        m_Encoder->setTransform(stack.m_Mtx, stack.m_Num);

        m_Encoder->setVertexBuffer(0, s_dds.m_vbh, mesh.m_startVertex, mesh.m_numVertices);
        m_Encoder->submit(m_ViewId, s_dds.m_program[_wireframe ? Program::Fill : Program::FillLit]);

        popTransform(false /* flush */);
    }

    void softFlush()
    {
        if (m_Pos == static_cast<uint16_t>(BX_COUNTOF(m_Cache)))
        {
            flush();
        }
    }

    void flush()
    {
        if (0 != m_Pos)
        {
            if (checkAvailTransientBuffers(m_Pos, DebugVertex::ms_layout, m_IndexPos))
            {
                bgfx::TransientVertexBuffer tvb;
                allocTransientVertexBuffer(&tvb, m_Pos, DebugVertex::ms_layout);
                bx::memCopy(tvb.data, m_Cache, m_Pos * DebugVertex::ms_layout.m_stride);

                bgfx::TransientIndexBuffer tib;
                allocTransientIndexBuffer(&tib, m_IndexPos);
                bx::memCopy(tib.data, m_Indices, m_IndexPos * sizeof(uint16_t));

                const Attrib& attrib = m_Attrib[m_Stack];

                m_Encoder->setVertexBuffer(0, &tvb);
                m_Encoder->setIndexBuffer(&tib);
                m_Encoder->setState(0
                    | BGFX_STATE_WRITE_RGB
                    | BGFX_STATE_PT_LINES
                    | attrib.m_state
                    | BGFX_STATE_LINEAA
                    | BGFX_STATE_BLEND_ALPHA
                );
                m_Encoder->setTransform(m_MtxStack[m_MtxStackCurrent].m_Mtx);
                const bgfx::ProgramHandle program = s_dds.m_program[attrib.m_stipple ? 1 : 0];
                m_Encoder->submit(m_ViewId, program);
            }

            m_State = State::None;
            m_Pos = 0;
            m_IndexPos = 0;
            m_VertexPos = 0;
        }
    }

    void flushQuad()
    {
        if (0 != m_PosQuad)
        {
            const uint32_t numIndices = m_PosQuad / 4 * 6;
            if (checkAvailTransientBuffers(m_PosQuad, DebugUvVertex::ms_layout, numIndices))
            {
                bgfx::TransientVertexBuffer tvb{};
                allocTransientVertexBuffer(&tvb, m_PosQuad, DebugUvVertex::ms_layout);
                bx::memCopy(tvb.data, m_CacheQuad, m_PosQuad * DebugUvVertex::ms_layout.m_stride);

                bgfx::TransientIndexBuffer tib{};
                allocTransientIndexBuffer(&tib, numIndices);
                auto* indices = reinterpret_cast<uint16_t*>(tib.data);
                for (uint16_t ii = 0, num = m_PosQuad / 4; ii < num; ++ii)
                {
                    const uint16_t startVertex = ii * 4;
                    indices[0] = startVertex + 0;
                    indices[1] = startVertex + 1;
                    indices[2] = startVertex + 2;
                    indices[3] = startVertex + 1;
                    indices[4] = startVertex + 3;
                    indices[5] = startVertex + 2;
                    indices += 6;
                }

                const Attrib& attrib = m_Attrib[m_Stack];

                m_Encoder->setVertexBuffer(0, &tvb);
                m_Encoder->setIndexBuffer(&tib);
                m_Encoder->setState(0
                    | (attrib.m_state & ~BGFX_STATE_CULL_MASK)
                );
                m_Encoder->setTransform(m_MtxStack[m_MtxStackCurrent].m_Mtx);
                m_Encoder->setTexture(0, s_dds.s_texColor, s_dds.m_texture);
                m_Encoder->submit(m_ViewId, s_dds.m_program[Program::FillTexture]);
            }

            m_PosQuad = 0;
        }
    }

    struct State
    {
        enum Enum
        {
            None,
            MoveTo,
            LineTo,

            Count
        };
    };

    static constexpr uint32_t c_kCacheSize = 1024;
    static constexpr uint32_t c_kStackSize = 16;
    static constexpr uint32_t c_kCacheQuadSize = 1024;
    BX_STATIC_ASSERT(c_kCacheSize >= 3, "Cache must be at least 3 elements.");

    DebugVertex m_Cache[c_kCacheSize + 1];
    DebugUvVertex m_CacheQuad[c_kCacheQuadSize];
    uint16_t m_Indices[c_kCacheSize * 2];
    uint16_t m_Pos;
    uint16_t m_PosQuad;
    uint16_t m_IndexPos;
    uint16_t m_VertexPos;
    uint32_t m_MtxStackCurrent;

    struct MatrixStack
    {
        void Reset()
        {
            m_Mtx = 0;
            m_Num = 1;
            m_Data = nullptr;
        }

        uint32_t m_Mtx;
        uint16_t m_Num;
        float* m_Data;
    };

    enum class DepthTest
    {
        Less,
        Greater,
        Always
    };

    MatrixStack m_MtxStack[32];

    bgfx::ViewId m_ViewId;
    uint8_t m_Stack;
    DepthTest m_DepthTest;

    Attrib m_Attrib[c_kStackSize];

    State::Enum m_State;

    bgfx::Encoder* m_Encoder;
    bgfx::Encoder* m_DefaultEncoder;
};

static DebugDrawEncoderImpl s_dde;

void atlas::render::debug::initialise()
{
    s_dds.init();
    s_dde.init(bgfx::begin());
}

void atlas::render::debug::shutdown()
{
    s_dde.shutdown();
    s_dds.shutdown();
}

atlas::render::debug::SpriteHandle ddCreateSprite(const uint16_t width, const uint16_t height, const void* data)
{
    return s_dds.createSprite(width, height, data);
}

void ddDestroy(atlas::render::debug::SpriteHandle _handle)
{
    s_dds.destroy(_handle);
}

atlas::render::debug::GeometryHandle ddCreateGeometry(
    const uint32_t numVertices,
    const atlas::render::debug::DdVertex* vertices,
    const uint32_t numIndices,
    const void* indices,
    const bool index32)
{
    return s_dds.createGeometry(numVertices, vertices, numIndices, indices, index32);
}

void ddDestroy(const atlas::render::debug::GeometryHandle handle)
{
    s_dds.destroy(handle);
}

void atlas::render::debug::debug_draw::begin(const uint16_t viewId, const bool depthTestLess, bgfx::Encoder* encoder)
{
    s_dde.begin(viewId, depthTestLess, encoder);
}

void atlas::render::debug::debug_draw::end()
{
    s_dde.end();
}

void atlas::render::debug::debug_draw::push()
{
    s_dde.push();
}

void atlas::render::debug::debug_draw::pop()
{
    s_dde.pop();
}

void atlas::render::debug::debug_draw::setDepthTestAlways()
{
    s_dde.setDepthTestAlways();
}

void atlas::render::debug::debug_draw::setDepthTestLess(const bool depthTestLess)
{
    s_dde.setDepthTestLess(depthTestLess);
}

void atlas::render::debug::debug_draw::setState(const bool depthTest, const bool depthWrite, const bool clockwise)
{
    s_dde.setState(depthTest, depthWrite, clockwise);
}

void atlas::render::debug::debug_draw::setColor(const core::Colour32 colour)
{
    s_dde.setColor(colour.GetAbgr());
}

void atlas::render::debug::debug_draw::setLod(const uint8_t lod)
{
    s_dde.setLod(lod);
}

void atlas::render::debug::debug_draw::setWireframe(const bool wireframe)
{
    s_dde.setWireframe(wireframe);
}

void atlas::render::debug::debug_draw::setStipple(const bool stipple, const float scale, const float offset)
{
    s_dde.setStipple(stipple, scale, offset);
}

void atlas::render::debug::debug_draw::setSpin(const float spin)
{
    s_dde.setSpin(spin);
}

void atlas::render::debug::debug_draw::setTransform(const void* mtx)
{
    s_dde.setTransform(mtx);
}

void atlas::render::debug::debug_draw::setTranslate(const float x, const float y, const float z)
{
    s_dde.setTranslate(x, y, z);
}

void atlas::render::debug::debug_draw::pushTransform(const void* mtx)
{
    s_dde.pushTransform(mtx, 1);
}

void atlas::render::debug::debug_draw::popTransform()
{
    s_dde.popTransform();
}

void atlas::render::debug::debug_draw::moveTo(const float x, const float y, const float z)
{
    s_dde.moveTo(x, y, z);
}

void atlas::render::debug::debug_draw::moveTo(const bx::Vec3& pos)
{
    s_dde.moveTo(pos);
}

void atlas::render::debug::debug_draw::lineTo(const float x, const float y, const float z)
{
    s_dde.lineTo(x, y, z);
}

void atlas::render::debug::debug_draw::lineTo(const bx::Vec3& pos)
{
    s_dde.lineTo(pos);
}

void atlas::render::debug::debug_draw::close()
{
    s_dde.close();
}

void atlas::render::debug::debug_draw::draw(const bx::Aabb& aabb)
{
    s_dde.draw(aabb);
}

void atlas::render::debug::debug_draw::draw(const bx::Cylinder& cylinder)
{
    s_dde.draw(cylinder, false);
}

void atlas::render::debug::debug_draw::draw(const bx::Capsule& capsule)
{
    s_dde.draw(*reinterpret_cast<const bx::Cylinder*>(&capsule), true);
}

void atlas::render::debug::debug_draw::draw(const bx::Disk& disk)
{
    s_dde.draw(disk);
}

void atlas::render::debug::debug_draw::draw(const bx::Obb& obb)
{
    s_dde.draw(obb);
}

void atlas::render::debug::debug_draw::draw(const bx::Sphere& sphere)
{
    s_dde.draw(sphere);
}

void atlas::render::debug::debug_draw::draw(const bx::Triangle& triangle)
{
    s_dde.draw(triangle);
}

void atlas::render::debug::debug_draw::draw(const bx::Cone& cone)
{
    s_dde.drawCone(cone.pos, cone.end, cone.radius);
}

void atlas::render::debug::debug_draw::drawFrustum(const void* viewProj)
{
    s_dde.drawFrustum(viewProj);
}

void atlas::render::debug::debug_draw::drawCircle(
    const bx::Vec3& normal,
    const bx::Vec3& center,
    const float radius,
    const float weight)
{
    s_dde.drawCircle(normal, center, radius, weight);
}

void atlas::render::debug::debug_draw::drawQuad(const bx::Vec3& normal, const bx::Vec3& center, const float size)
{
    s_dde.drawQuad(normal, center, size);
}

void atlas::render::debug::debug_draw::drawQuad(
    const bgfx::TextureHandle handle,
    const bx::Vec3& normal,
    const bx::Vec3& center,
    const float size)
{
    s_dde.drawQuad(handle, normal, center, size);
}

void atlas::render::debug::debug_draw::drawCone(const bx::Vec3& from, const bx::Vec3& to, const float radius)
{
    s_dde.drawCone(from, to, radius);
}

void atlas::render::debug::debug_draw::drawAxis(
    const float x,
    const float y,
    const float z,
    const float len,
    const float thickness)
{
    s_dde.drawAxis(x, y, z, len, Axis::Count, thickness);
}

void atlas::render::debug::debug_draw::drawCylinder(
    const bx::Vec3& from,
    const bx::Vec3& to,
    const float radius)
{
    s_dde.drawCylinder(from, to, radius, false);
}

void atlas::render::debug::debug_draw::drawCapsule(
    const bx::Vec3& from,
    const bx::Vec3& to,
    const float radius)
{
    s_dde.drawCylinder(from, to, radius, true);
}

void atlas::render::debug::debug_draw::drawGrid(
    const bx::Vec3& normal,
    const bx::Vec3& center,
    const uint32_t size,
    const float step)
{
    s_dde.drawGrid(normal, center, size, step);
}

atlas::render::debug::DebugDrawEncoderScopePush atlas::render::debug::debug_draw::createScope()
{
    return {};
}

atlas::render::debug::DebugDrawEncoderScopePush::DebugDrawEncoderScopePush()
{
    debug_draw::push();
}

atlas::render::debug::DebugDrawEncoderScopePush::~DebugDrawEncoderScopePush()
{
    debug_draw::pop();
}
