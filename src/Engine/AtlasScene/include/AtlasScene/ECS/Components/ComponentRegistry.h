#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "Pools.h"
#include "AtlasCore/TemplatedUniquenessCounter.h"

namespace atlas::scene
{
    class ComponentRegistry
    {
    public:
        struct ComponentField
        {
            std::string_view m_Name;
            std::string_view m_Type;
            std::size_t m_Offset;
            std::size_t m_Size;
        };

        struct ComponentRegistration
        {
            int32_t m_UniqueId;
            std::string_view m_ComponentName;

            std::vector<ComponentField> m_Fields;
        };

        using PoolFactory = PoolBase* (*)(void);

        ComponentRegistry() = delete;
        ~ComponentRegistry() = delete;
        ComponentRegistry(const ComponentRegistry&) = delete;
        ComponentRegistry(const ComponentRegistry&&) = delete;
        ComponentRegistry operator=(const ComponentRegistry&) = delete;
        ComponentRegistry operator=(const ComponentRegistry&&) = delete;

        template <typename TComponent>
        static void RegisterComponent(ComponentRegistration&& registration)
        {
            std::lock_guard lock{m_ComponentRegistrationMutex};
            registration.m_UniqueId = core::TemplatedUniquenessCounter<TComponent, ComponentRegistry>::Ensure();

            m_ComponentRegistrations.emplace_back(registration);
            m_ComponentPoolFactory.push_back([] { return static_cast<PoolBase*>(new ComponentPool<TComponent>()); });
        }

        static const std::vector<PoolFactory>& GetComponentPoolFactories()
        {
            return m_ComponentPoolFactory;
        }

        static const PoolFactory& GetFactoryForPoolWithMask(const uint64_t mask)
        {
            auto index = std::countr_zero(mask);
            return m_ComponentPoolFactory[index];
        }

        template <typename TComponent>
        static uint64_t GetComponentMask()
        {
            const uint64_t index = core::TemplatedUniquenessCounter<TComponent, ComponentRegistry>::GetTypeValue();
            assert(index != -1); // Component has not been registered
            return 1ULL << index;
        }

        static std::mutex& GetComponentRegistrationMutex()
        {
            return m_ComponentRegistrationMutex;
        }

        static const std::vector<ComponentRegistration>& GetComponentRegistrations()
        {
            return m_ComponentRegistrations;
        }

    private:
        inline static std::mutex m_ComponentRegistrationMutex;
        inline static std::vector<ComponentRegistration> m_ComponentRegistrations;
        inline static std::vector<PoolFactory> m_ComponentPoolFactory;
    };
}
