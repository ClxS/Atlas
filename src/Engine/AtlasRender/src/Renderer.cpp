#include "AtlasRenderPCH.h"
#include "Renderer.h"

#include "bgfx/platform.h"

#include <functional>

#include "DebugDraw.h"
#include <imgui.h>
#include <ImGuizmo.h>

namespace
{
    struct Task
    {
        std::string m_Name;
        std::function<void()> m_Callback;
        bool m_IsOneOff;
    };

    std::vector<Task> g_RenderMethods;
    bgfx::ViewId g_DebugGeometryView;
}

void initDrawingData();

void atlas::render::init(const RendererInitArgs& args)
{
    bgfx::PlatformData pd{};
    pd.nwh = args.m_WindowHandle;
#if BX_PLATFORM_LINUX
#error implement ndt
#endif

    bgfx::Init bgfxInitArgs;
    bgfxInitArgs.type = bgfx::RendererType::Count; // auto choose renderer
    bgfxInitArgs.resolution.width = args.m_Width;
    bgfxInitArgs.resolution.height = args.m_Height;
    bgfxInitArgs.resolution.reset = BGFX_RESET_VSYNC;
    bgfxInitArgs.platformData = pd;
    bgfx::init(bgfxInitArgs);

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, args.m_Width, args.m_Height);

    g_DebugGeometryView = args.m_DebugGeometryView;
    debug::initialise();

    initDrawingData();
}

atlas::render::RenderTaskHandle atlas::render::addToFrameGraph(
    std::string_view name,
    std::function<void()> initiailize,
    std::function<void()> callback,
    std::vector<RenderTaskHandle> dependentTasks)
{
    g_RenderMethods.emplace_back(std::string(name), initiailize, true);
    g_RenderMethods.emplace_back(std::string(name), callback, false);
    return {-1};
}

atlas::render::RenderTaskHandle atlas::render::addToFrameGraph(
    std::string_view name,
    std::function<void()> callback,
    std::vector<RenderTaskHandle> dependentTasks)
{
    g_RenderMethods.emplace_back(std::string(name), callback, false);
    return {-1};
}

atlas::render::RenderTaskHandle atlas::render::addToFrameGraph_oneOff(std::string_view name, std::function<void()> callback,
    std::vector<RenderTaskHandle> dependentTasks)
{
    g_RenderMethods.emplace_back(std::string(name), callback, true);
    return {-1};
}

void atlas::render::sync()
{
    bgfx::setState(BGFX_STATE_DEFAULT);

    debug::debug_draw::begin(g_DebugGeometryView);

    for(auto& task : g_RenderMethods)
    {
        if (task.m_Callback)
        {
            bgfx::setMarker((task.m_Name + "_Begin").c_str());
            task.m_Callback();
            bgfx::setMarker((task.m_Name + "_End").c_str());
        }
    }

    debug::debug_draw::end();
    bgfx::frame();
    std::erase_if(g_RenderMethods, [](const Task& t) { return t.m_IsOneOff; });
}
