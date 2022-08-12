#pragma once
#include <algorithm>
#include <AtlasScene/SceneManager.h>

#include <SDL_syswm.h>
#include <AtlasAppHost/Application.h>
#include <AtlasAppHost/Main.h>
#include <AtlasGame/GameHost.h>
#include <AtlasResource/ResourceLoader.h>
#include <bgfx/platform.h>

#include "AtlasUI/Rml/RmlConfig.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/AssetTypes/MeshAsset.h"
#include "AtlasRender/AssetTypes/ModelAsset.h"
#include "AtlasTrace/Logging.h"
#include "Utility/FrameLimiter.h"

namespace atlas::game
{
    class GameImplementation
    {
    public:
        GameImplementation() = default;
        virtual ~GameImplementation() {}
        GameImplementation(const GameImplementation&) = delete;
        GameImplementation(const GameImplementation&&) = delete;
        GameImplementation operator=(const GameImplementation&) = delete;
        GameImplementation operator=(const GameImplementation&&) = delete;

        virtual void OnStartup() {}
        virtual void Tick()
        {
            m_SceneManager.Update();
        }

    protected:
        scene::SceneManager m_SceneManager;
    };

    template<typename TGameImplementation>
    class GameHost
    {
    public:
        enum class ReturnCode
        {
            ReturnCode_Success = 0,
            ReturnCode_ApplicationFailed = -1,
            ReturnCode_RendererFailed = -2,
            ReturnCode_ResourceSystemFailed = -3,
            ReturnCode_UIFailed = -4,
        };

        struct Args
        {
            std::string m_GameName;
            bgfx::ViewId m_UIView = 0;
            int m_FrameRateCap = 60;
        };

        explicit GameHost(Args args = {})
            : m_GameArguments{std::move(args)}
        {
        }

        bool InitialiseRenderer() const
        {
            AT_INFO(AtlasGame, "Initialising renderer");
            auto& app = app_host::Application::Get();

#if !BX_PLATFORM_EMSCRIPTEN
            const auto& platform = app.GetPlatform();
            const auto dimensions = platform.GetAppDimensions();
            auto window = app_host::Application::Get().GetPlatform().GetSDLContext().m_Window;
            SDL_SysWMinfo wmi;
            SDL_VERSION(&wmi.version);
            if (!SDL_GetWindowWMInfo(window, &wmi)) {
                printf(
                    "SDL_GetWindowWMInfo could not be retrieved. SDL_Error: %s\n",
                    SDL_GetError());
                return false;
            }

            bgfx::renderFrame(); // single threaded mode
            #endif // !BX_PLATFORM_EMSCRIPTEN

            render::RendererInitArgs args;
            args.m_Width = std::get<0>(dimensions);
            args.m_Height = std::get<1>(dimensions);

#if BX_PLATFORM_WINDOWS
            args.m_WindowHandle = wmi.info.win.window;
#elif BX_PLATFORM_OSX
            args.m_WindowHandle = wmi.info.cocoa.window;
#elif BX_PLATFORM_LINUX
            args.m_WindowHandle = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_EMSCRIPTEN
            args.m_WindowHandle = (void*)"#canvas";
#endif

            init(args);
            return true;
        }

        static bool InitialiseUI(const bgfx::ViewId uiView)
        {
            AT_INFO(AtlasGame, "Initialising UI");
            return ui::rml::config::initialiseRmlUi(uiView);
        }

        bool InitialiseResourceSystem() const
        {
            resource::ResourceLoader::RegisterTypeHandler<render::VertexShader>(render::vertexShaderLoadHandler);
            resource::ResourceLoader::RegisterTypeHandler<render::FragmentShader>(render::fragmentShaderLoadHandler);
            resource::ResourceLoader::RegisterTypeHandler<render::ShaderProgram>(render::shaderProgramLoadHandler);
            resource::ResourceLoader::RegisterTypeHandler<render::TextureAsset>(render::textureLoadHandler);
            resource::ResourceLoader::RegisterTypeHandler<render::ModelAsset>(render::modelLoadHandler);
            resource::ResourceLoader::RegisterTypeHandler<render::MeshAsset>(render::meshLoadHandler);

            return true;
        }

        int Run()
        {
            AT_INFO(AtlasGame, "Initialising Game... {}", 3434);

            if (!app_host::Application::Get().Initialise(m_GameArguments.m_GameName))
            {
                AT_ERROR(AtlasGame, "Failed to initialise application");
                return static_cast<int>(ReturnCode::ReturnCode_ApplicationFailed);
            }

            logStartUp();
            srand(static_cast<unsigned>(time(nullptr)));
            auto [iWidth, iHeight] = app_host::Application::Get().GetAppDimensions();

            utility::FrameLimiter frameLimiter(m_GameArguments.m_FrameRateCap);
            frameLimiter.Start();

            if (!InitialiseRenderer())
            {
                return static_cast<int>(ReturnCode::ReturnCode_RendererFailed);
            }

            if (!InitialiseResourceSystem())
            {
                return static_cast<int>(ReturnCode::ReturnCode_ResourceSystemFailed);
            }

            if (!InitialiseUI(m_GameArguments.m_UIView))
            {
                return static_cast<int>(ReturnCode::ReturnCode_UIFailed);
            }

            m_Game.OnStartup();

            auto& app = app_host::Application::Get();
            while(true)
            {
                bgfx::touch(0);

                app.Update();
                m_Game.Tick();

                render::sync();
                frameLimiter.Limit();
                frameLimiter.EndFrame();
            }

            return static_cast<int>(ReturnCode::ReturnCode_Success);
        }

    private:
        Args m_GameArguments;
        TGameImplementation m_Game;
    };
}
