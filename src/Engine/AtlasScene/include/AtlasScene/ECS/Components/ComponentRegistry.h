#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "Pools.h"
#include "AtlasCore/TemplatedUniquenessCounter.h"

namespace atlas::scene
{
    DEFINE_ID_CONTAINER(ComponentInfoId, int32_t);
    DEFINE_ID_CONTAINER(ComponentFieldInfoId, int32_t);

    class ComponentRegistry
    {
    public:
        struct ComponentFieldInfo
        {
            std::string_view m_Name;
            std::string_view m_Type;
            std::size_t m_Offset;
            std::size_t m_Size;
            ComponentFieldInfoId m_FieldId;
        };

        struct ComponentInfo
        {
            ComponentInfoId m_UniqueId;
            std::string_view m_ComponentName;

            std::vector<ComponentInfoId> m_RequiredComponents;
            std::vector<ComponentFieldInfo> m_Fields;

            const ComponentFieldInfo& GetFieldInfo(const ComponentFieldInfoId& fieldId) const
            {
                assert(fieldId.IsValid() && fieldId.m_Value < m_Fields.size());
                return m_Fields[fieldId.m_Value];
            }

#if defined(DEBUG)
            void ValidateFieldOffsets()
            {
                for(int32_t i = 0; i < (int32_t)m_Fields.size(); i++)
                {
                    assert(i == m_Fields[i].m_FieldId);
                }
            }
#endif
        };

        using PoolFactory = PoolBase* (*)();

        ComponentRegistry() = delete;
        ~ComponentRegistry() = delete;
        ComponentRegistry(const ComponentRegistry&) = delete;
        ComponentRegistry(const ComponentRegistry&&) = delete;
        ComponentRegistry operator=(const ComponentRegistry&) = delete;
        ComponentRegistry operator=(const ComponentRegistry&&) = delete;

        static const ComponentInfo& GetComponentInfo(const ComponentInfoId& componentId)
        {
            assert(componentId.IsValid() && componentId.m_Value < m_ComponentRegistrations.size());
            return m_ComponentRegistrations[componentId.m_Value];
        }

        template <typename TComponent>
        static void RegisterComponent(ComponentInfo&& registration)
        {
            std::lock_guard lock{m_ComponentRegistrationMutex};
            registration.m_UniqueId = ComponentInfoId { core::TemplatedUniquenessCounter<TComponent, ComponentRegistry>::Ensure() };

#if defined(DEBUG)
            registration.ValidateFieldOffsets();
#endif

            // Check the assumption that the UniqueId maps with the registration index
            assert(m_ComponentRegistrations.size() == registration.m_UniqueId.m_Value);
            m_ComponentRegistrations.emplace_back(registration);
            m_ComponentPoolFactory.push_back([] { return static_cast<PoolBase*>(new ComponentPool<TComponent>()); });
        }

        static const std::vector<PoolFactory>& GetComponentPoolFactories()
        {
            return m_ComponentPoolFactory;
        }

        static const PoolFactory& GetFactoryForPoolWithMask(const uint64_t mask)
        {
            const auto index = std::countr_zero(mask);
            return m_ComponentPoolFactory[index];
        }

        template <typename TComponent>
        static ComponentInfoId GetComponentId()
        {
            return ComponentInfoId { core::TemplatedUniquenessCounter<TComponent, ComponentRegistry>::GetTypeValue() };
        }

        template <typename TComponent>
        static uint64_t GetComponentMask()
        {
            return GetComponentMask(GetComponentId<TComponent>());
        }

        static uint64_t GetComponentMask(const ComponentInfoId componentId)
        {
            assert(componentId.IsValid()); // Component has not been registered
            return 1ULL << componentId.m_Value;
        }

        static std::mutex& GetComponentRegistrationMutex()
        {
            return m_ComponentRegistrationMutex;
        }

        static const std::vector<ComponentInfo>& GetComponentRegistrations()
        {
            return m_ComponentRegistrations;
        }

    private:
        inline static std::mutex m_ComponentRegistrationMutex;
        inline static std::vector<ComponentInfo> m_ComponentRegistrations;
        inline static std::vector<PoolFactory> m_ComponentPoolFactory;
    };
}
