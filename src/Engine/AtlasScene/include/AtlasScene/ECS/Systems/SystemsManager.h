#pragma once

#include <algorithm>
#include <bitset>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <ranges>
#include <vector>

#include "SystemBase.h"
#include "AtlasCore/TemplatedUniquenessCounter.h"

namespace atlas::scene
{
    class SystemsBuilder;

    class SystemsBuilder
    {
    public:
        ~SystemsBuilder()
        {
            for (const auto& system : m_Systems)
            {
                if (system.m_pSystem.has_value())
                {
                    delete system.m_pSystem.value();
                }
            }

            m_Systems.clear();
        }

        struct GroupId
        {
            int32_t m_GroupId = 0;
        };

        template <typename TSystem, typename ... DependsOn, typename... TArgs>
        TSystem* RegisterSystem(std::vector<GroupId> dependentGroups, TArgs&&...);

        template <typename TSystem, typename ... DependsOn, typename... TArgs>
        TSystem* RegisterSystem(TArgs&&...);

        void RegisterLambda(const std::string& systemName, std::function<void()>&& update)
        {
            m_Systems.push_back(
                SystemRegistration {
                    systemName,
                    std::move(update),
                    GroupId {0},
                    0,
                    {}});
        }

        void RegisterLambda(const std::string& systemName, std::function<void()>&& initialise, std::function<void()>&& update)
        {
            m_Systems.push_back(
                SystemRegistration {
                    systemName,
                    std::move(initialise),
                    std::move(update),
                    GroupId {0},
                    0,
                    {}});
        }

        template <typename ... DependsOn>
        GroupId RegisterGroup(std::string, std::function<void(SystemsBuilder&)> callback);

        template <typename ... DependsOn>
        GroupId RegisterGroup(std::string, std::vector<GroupId> dependentGroups,
                              std::function<void(SystemsBuilder&)> callback);


    private:
        friend class SystemsManager;

        template <typename TSystem>
        int32_t GetSystemIndex();
        int32_t GetMinimumDependencyIndex(int32_t uniquenessValue) const;
        int32_t GetMaximumGroupIndex(std::vector<GroupId> dependentGroups) const;

        struct SystemRegistration
        {
            SystemRegistration(std::string systemName, SystemBase* pSystem, GroupId groupId, int32_t value,
                               std::vector<int32_t> dependencies);

            SystemRegistration(std::string systemName, std::function<void()>&& update, GroupId groupId, int32_t value,
                               std::vector<int32_t> dependencies);

            SystemRegistration(std::string systemName, std::function<void()>&& initialise, std::function<void()>&& update, GroupId groupId, int32_t value,
                               std::vector<int32_t> dependencies);

            std::string m_SystemName;
            std::optional<SystemBase*> m_pSystem;
            std::optional<std::function<void()>> m_LambdaInitialise;
            std::optional<std::function<void()>> m_LambdaUpdate;
            GroupId m_GroupId;
            int32_t m_UniquenessValue;
            std::vector<int32_t> m_Dependencies;
        };

        std::vector<SystemRegistration> m_Systems;
        int32_t m_LatestGroupId = 0;
    };

    class SystemsManager
    {
    public:
        ~SystemsManager()
        {
            for (const auto& system : m_Systems)
            {
                delete system;
            }

            m_Systems.clear();
        }

        void Initialise(SystemsBuilder& builder, EcsManager& ecsManager)
        {
            std::cout << "Systems Order:";
            for (auto& system : builder.m_Systems)
            {
                std::cout << std::format("\t{}", system.m_SystemName);
                if (system.m_pSystem.has_value())
                {
                    m_Systems.push_back(system.m_pSystem.value());
                }
            }

            builder.m_Systems.clear();
            for (const auto& system : m_Systems)
            {
                system->Initialise(ecsManager);
            }
        }

        void InitialiseDeferred(SystemsBuilder& builder, EcsManager& ecsManager)
        {
            std::cout << "Systems Order:";
            for (auto& system : builder.m_Systems)
            {
                std::cout << std::format("\t{}", system.m_SystemName);
                if (system.m_pSystem.has_value())
                {
                    m_Systems.push_back(system.m_pSystem.value());
                }

                if (system.m_LambdaUpdate.has_value())
                {
                    if (system.m_LambdaInitialise.has_value())
                    {
                        m_Systems.push_back(new LambdaWrapper(
                            system.m_SystemName,
                            system.m_LambdaInitialise.value(),
                            system.m_LambdaUpdate.value()));
                    }
                    else
                    {
                        m_Systems.push_back(new LambdaWrapper(
                            system.m_SystemName,
                            system.m_LambdaUpdate.value()));
                    }
                }
            }

            builder.m_Systems.clear();
        }

        void Update(EcsManager& ecsManager) const
        {
            for (const auto& system : m_Systems)
            {
                Update(ecsManager, system);
            }
        }

        static void Update(EcsManager& ecsManager, SystemBase* system)
        {
            system->Update(ecsManager);
        }

        [[nodiscard]] const std::vector<SystemBase*>& GetSystems() const { return m_Systems; }

    private:
        class LambdaWrapper : public SystemBase
        {
        public:
            LambdaWrapper(std::string& name, std::function<void()> update)
                : m_Name{name}
                , m_Update{update}
            {
            }

            LambdaWrapper(std::string& name, std::function<void()> initialise, std::function<void()> update)
                : m_Name{name}
                , m_Initialise{initialise}
                , m_Update{update}
            {
            }

            void Initialise(EcsManager&) override
            {
                if (m_Initialise.has_value())
                {
                    m_Initialise.value()();
                }
            }

            void Render(EcsManager&) override
            {
                if (m_Update.has_value())
                {
                    m_Update.value()();
                }
            }

            [[nodiscard]] std::string_view GetName() const override
            {
                return m_Name;
            }

        private:
            std::string m_Name;
            std::optional<std::function<void()>> m_Initialise;
            std::optional<std::function<void()>> m_Update;
        };

        std::vector<SystemBase*> m_Systems;
    };

    template <typename TSystem>
    int32_t SystemsBuilder::GetSystemIndex()
    {
        // Slow but infrequently used
        const int32_t uniquenessValue = core::TemplatedUniquenessCounter<TSystem, SystemsBuilder>::GetTypeValue();
        for (size_t i = 0; i < m_Systems.size(); ++i)
        {
            if (m_Systems[i].m_UniquenessValue == uniquenessValue)
            {
                return static_cast<int32_t>(i);
            }
        }

        return -1;
    }

    inline int32_t SystemsBuilder::GetMinimumDependencyIndex(const int32_t uniquenessValue) const
    {
        for (size_t i = 0; i < m_Systems.size(); ++i)
        {
            for (const auto dependencyValue : m_Systems[i].m_Dependencies)
            {
                if (dependencyValue == uniquenessValue)
                {
                    return static_cast<int32_t>(i);
                }
            }
        }

        return static_cast<int32_t>(m_Systems.size());
    }

    inline int32_t SystemsBuilder::GetMaximumGroupIndex(const std::vector<GroupId> dependentGroups) const
    {
        int32_t outValue = -1;
        for (const auto& group : dependentGroups)
        {
            for (int32_t i = static_cast<int32_t>(m_Systems.size()) - 1; i >= 0; i--)
            {
                if (m_Systems[i].m_GroupId.m_GroupId == group.m_GroupId)
                {
                    outValue = std::max(outValue, i);
                }
            }
        }
        return outValue;
    }

    template <typename TSystem, typename ... DependsOn, typename... TArgs>
    TSystem* SystemsBuilder::RegisterSystem(TArgs&&... args)
    {
        return RegisterSystem<TSystem, DependsOn..., TArgs...>({}, std::forward<TArgs&&>(args)...);
    }

    template <typename TSystem, typename ... DependsOn, typename... TArgs>
    TSystem* SystemsBuilder::RegisterSystem(std::vector<GroupId> dependentGroups, TArgs&&... args)
    {
        core::TemplatedUniquenessCounter<TSystem, SystemsBuilder>::Ensure();

        std::vector<int32_t> dependencyIndices{GetSystemIndex<DependsOn>()...};
        std::erase(dependencyIndices, -1);

        int32_t minimumIndex = dependencyIndices.empty() ? -1 : *std::ranges::max_element(dependencyIndices);
        minimumIndex = std::max(minimumIndex, GetMaximumGroupIndex(dependentGroups));

        const int32_t maximumIndex = GetMinimumDependencyIndex(
            core::TemplatedUniquenessCounter<TSystem, SystemsBuilder>::GetTypeValue());

        assert(minimumIndex < maximumIndex); // If we hit this, we have a circular dependency issue

        TSystem* pSystem = new TSystem(std::forward<TArgs&&>(args)...);
        m_Systems.insert(
            m_Systems.begin() + maximumIndex,
            SystemRegistration(
                typeid(TSystem).name(),
                pSystem,
                {0},
                core::TemplatedUniquenessCounter<TSystem, SystemsManager>::Ensure(),
                {core::TemplatedUniquenessCounter<DependsOn, SystemsManager>::GetTypeValue()...}));
        return pSystem;
    }

    template <typename ... DependsOn>
    SystemsBuilder::GroupId SystemsBuilder::RegisterGroup(std::string groupName,
                                                          std::function<void(SystemsBuilder&)> callback)
    {
        return RegisterGroup<DependsOn...>(groupName, {}, callback);
    }

    template <typename ... DependsOn>
    SystemsBuilder::GroupId SystemsBuilder::RegisterGroup(std::string, std::vector<GroupId> dependentGroups,
                                                          std::function<void(SystemsBuilder&)> callback)
    {
        SystemsBuilder groupBuilder;
        callback(groupBuilder);
        if (groupBuilder.m_Systems.empty())
        {
            return {};
        }

        std::vector<int32_t> dependencyIndices{GetSystemIndex<DependsOn>()...};
        std::erase(dependencyIndices, -1);

        int32_t minimumIndex = dependencyIndices.empty() ? -1 : *std::ranges::max_element(dependencyIndices);
        minimumIndex = std::max(minimumIndex, GetMaximumGroupIndex(dependentGroups));

        int32_t maximumIndex = 0;
        for (const auto& registration : groupBuilder.m_Systems)
        {
            maximumIndex = std::max(maximumIndex, GetMinimumDependencyIndex(registration.m_UniquenessValue));
        }

        assert(minimumIndex < maximumIndex); // If we hit this, we have a circular dependency issue

        const GroupId groupId = {++m_LatestGroupId};
        for (const auto& registration : groupBuilder.m_Systems | std::views::reverse)
        {
            m_Systems.insert(
                m_Systems.begin() + maximumIndex,
                SystemRegistration(
                    registration.m_SystemName,
                    registration.m_pSystem,
                    groupId,
                    registration.m_UniquenessValue,
                    registration.m_Dependencies));
        }

        groupBuilder.m_Systems.clear();
        return groupId;
    }

    inline SystemsBuilder::SystemRegistration::SystemRegistration(
        std::string systemName,
        SystemBase* pSystem,
        GroupId groupId,
        const int32_t value,
        std::vector<int32_t> dependencies)
          : m_SystemName{std::move(systemName)}
          , m_pSystem{pSystem}
          , m_GroupId{groupId}
          , m_UniquenessValue{value}
          , m_Dependencies{std::move(dependencies)}
    {
    }

    inline SystemsBuilder::SystemRegistration::SystemRegistration(
        std::string systemName,
        std::function<void()>&& update,
        GroupId groupId,
        const int32_t value,
        std::vector<int32_t> dependencies)
          : m_SystemName{std::move(systemName)}
          , m_LambdaUpdate{update}
          , m_GroupId{groupId}
          , m_UniquenessValue{value}
          , m_Dependencies{std::move(dependencies)}
    {
    }

    inline SystemsBuilder::SystemRegistration::SystemRegistration(
        std::string systemName,
        std::function<void()>&& initialise,
        std::function<void()>&& update,
        GroupId groupId,
        const int32_t value,
        std::vector<int32_t> dependencies)
          : m_SystemName{std::move(systemName)}
          , m_LambdaInitialise{initialise}
          , m_LambdaUpdate{update}
          , m_GroupId{groupId}
          , m_UniquenessValue{value}
          , m_Dependencies{std::move(dependencies)}
    {
    }
}
