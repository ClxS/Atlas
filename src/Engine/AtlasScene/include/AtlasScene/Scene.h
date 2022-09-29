#pragma once

#include "AtlasRender/Renderer.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "ECS/Systems/SystemsManager.h"

namespace atlas::scene
{
    class SceneManager;

    class SceneBase
    {
    public:
        virtual ~SceneBase() = default;

        virtual void OnEntered(SceneManager& sceneManager)
        {
        }

        virtual void OnUpdate(SceneManager& sceneManager)
        {
        }

        virtual void OnExited(SceneManager& sceneManager)
        {
        }
    };

    class EcsScene : public SceneBase
    {
    public:
        EcsManager& GetEcsManager() { return m_EcsManager; }

    protected:
        ~EcsScene() override = default;

        void OnEntered(SceneManager& sceneManager) override
        {
            SystemsBuilder simBuilder;
            SystemsBuilder renderBuilder;
            ConstructSystems(simBuilder, renderBuilder);

            m_SimulationSystemsManager.Initialise(simBuilder, m_EcsManager);
            m_RenderSystemsManager.InitialiseDeferred(renderBuilder, m_EcsManager);

            for(auto& system : m_RenderSystemsManager.GetSystems())
            {
                m_RenderTaskHandles.emplace_back(render::addToFrameGraph(
                    system->GetName(),
                    [this, system]() { system->Initialise(m_EcsManager); },
                    [this, system]() { system->Render(m_EcsManager); }));
            }
        }

        void OnExited(SceneManager& sceneManager) override
        {
            // TODO Remove all render tasks
        }

        void OnUpdate(SceneManager& sceneManager) override
        {
            m_SimulationSystemsManager.Update(m_EcsManager);
        }

        template<typename TSystem, typename... TArgs>
        void DirectInitialiseSystem(TSystem& system, TArgs&&... args)
        {
            system.Initialise(m_EcsManager, std::forward<TArgs>(args)...);
        }

        void DirectRunSystem(SystemBase& system)
        {
            SystemsManager::Update(m_EcsManager, &system);
        }

        virtual void ConstructSystems(SystemsBuilder& simBuilder, SystemsBuilder& frameBuilder) = 0;


    private:
        SystemsManager m_SimulationSystemsManager;
        SystemsManager m_RenderSystemsManager;
        std::vector<atlas::render::RenderTaskHandle> m_RenderTaskHandles;
        EcsManager m_EcsManager;
    };
}
