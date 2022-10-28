#include "AtlasAppHostPCH.h"
#include "AtlasAppHost/Application.h"
#include "AtlasAppHost/PlatformApplication.h"
#include "AtlasCore/CommandLine.h"

#include "ApplicationArguments.h"
#include "SDL.h"
#include "SDL_syswm.h"
#include "backends/imgui_impl_sdl.h"

#define WINDOWED 1

DEFINE_COMMAND_ARG_L(IsEmbedded, bool, "embedded", "");
DEFINE_COMMAND_ARG_L(HostWindowHandle, int32_t, "host_handle", "");

namespace atlas::input
{
    extern void clearMouseWheel_private();
    extern void setMouseWheel_private(float x, float y);
}

bool atlas::app_host::platform::PlatformApplication::Initialise(const ApplicationArguments& applicationName)
{
    int windowFlags;

    constexpr int rendererFlags = SDL_RENDERER_ACCELERATED;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    const std::string title {applicationName.m_ApplicationName};
#if WINDOWED
    windowFlags = SDL_WINDOW_SHOWN;
    windowFlags |= SDL_WINDOW_RESIZABLE;

    if (command_line::GetIsEmbedded() && command_line::GetHostWindowHandle() != 0)
    {
        const size_t handle = command_line::GetHostWindowHandle();

        const HWND targetHwnd = reinterpret_cast<HWND>(handle);
        RECT targetRect;
        GetWindowRect(targetHwnd, &targetRect);
        const int targetWidth = targetRect.right - targetRect.left;
        const int targetHeight = targetRect.bottom - targetRect.top;

        windowFlags &= ~SDL_WINDOW_SHOWN;
        windowFlags |= SDL_WINDOW_HIDDEN;
        windowFlags |= SDL_WINDOW_BORDERLESS;
        windowFlags |= SDL_WINDOW_RESIZABLE;
        m_Sdl.m_Window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            targetWidth,
            targetHeight,
            windowFlags);

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version)
        SDL_GetWindowWMInfo(m_Sdl.m_Window, &wmInfo);
        const HWND ownHwnd = wmInfo.info.win.window;
        m_Sdl.m_OwnHwnd = ownHwnd;
        m_Sdl.m_TrackedParentHwnd = targetHwnd;

        SetParent(ownHwnd, targetHwnd);
        ShowWindow(ownHwnd, SW_SHOW);
        MoveWindow(ownHwnd, 0, 0, targetRect.right - targetRect.left, targetRect.bottom - targetRect.top, TRUE);
    }
    else
    {
        m_Sdl.m_Window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 800,
                                          windowFlags);
    }
#else
    windowFlags = 0;
    m_Sdl.m_Window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 2560, 1080, windowFlags);
    SDL_SetRelativeMouseMode(SDL_TRUE);
#endif

    if (!m_Sdl.m_Window)
    {
        printf("Failed to open %d x %d window: %s\n", 800, 600, SDL_GetError());
        return false;
    }

    ImGui::CreateContext();
    return true;
}

std::tuple<int, int> atlas::app_host::platform::PlatformApplication::GetAppDimensions() const
{
    int width, height;
    SDL_GetWindowSize(m_Sdl.m_Window, &width, &height);

    return std::make_tuple(width, height);
}

void atlas::app_host::platform::PlatformApplication::Update()
{
    if (m_Sdl.m_TrackedParentHwnd && m_Sdl.m_OwnHwnd)
    {
        HWND targetHwnd = static_cast<HWND>(m_Sdl.m_TrackedParentHwnd);
        RECT rc;
        // Get the position of the window relative to the entire screen
        GetWindowRect(targetHwnd, &rc);

        const int width = rc.right - rc.left;
        const int height = rc.bottom - rc.top;

        int currentWidth;
        int currentHeight;
        SDL_GetWindowSize(m_Sdl.m_Window, &currentWidth, &currentHeight);
        if (currentWidth != width || currentHeight != height)
        {
            SDL_SetWindowSize(m_Sdl.m_Window, width, height);
            MoveWindow(static_cast<HWND>(m_Sdl.m_OwnHwnd), 0, 0, width, height, TRUE);
        }
    }

    input::clearMouseWheel_private();

    SDL_Event currentEvent;
    while(SDL_PollEvent(&currentEvent) != 0)
    {
        ImGui_ImplSDL2_ProcessEvent(&currentEvent);
        switch (currentEvent.type)
        {
        case SDL_QUIT:
            exit(0);
            break;
        case SDL_MOUSEWHEEL:
            atlas::input::setMouseWheel_private(currentEvent.wheel.preciseX, currentEvent.wheel.preciseY);
            break;
        default:
            break;
        }

        // TODO
        if (currentEvent.type == SDL_KEYDOWN)
        {
            int i = 0;
            i++;
        }
    }

    SDL_PumpEvents();
}
