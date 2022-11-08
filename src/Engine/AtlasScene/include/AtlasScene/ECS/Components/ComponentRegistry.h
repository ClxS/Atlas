#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <array>

#include "Pools.h"
#include "AtlasCore/TemplatedUniquenessCounter.h"
#include <span>

namespace atlas::scene
{
    DEFINE_ID_CONTAINER(ComponentInfoId, int32_t);
    DEFINE_ID_CONTAINER(ComponentFieldInfoId, int32_t);

    class ComponentRegistry
    {
    public:
        static constexpr int c_maxComponents = 256;

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

            const auto idx = registration.m_UniqueId.m_Value;
            assert(idx < c_maxComponents);
            m_ComponentRegistrations[idx] = registration;
            m_ComponentPoolFactory[idx] = ([] { return static_cast<PoolBase*>(new ComponentPool<TComponent>()); });
            m_ComponentCount = idx + 1;
        }

        static const std::span<PoolFactory> GetComponentPoolFactories()
        {
            return { m_ComponentPoolFactory };
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

        static std::span<const ComponentInfo> GetComponentRegistrations()
        {
            return { m_ComponentRegistrations.data(), static_cast<size_t>(m_ComponentCount) };
        }

    private:
        inline static std::mutex m_ComponentRegistrationMutex;
        inline static std::array<ComponentInfo, c_maxComponents> m_ComponentRegistrations;
        inline static std::array<PoolFactory, c_maxComponents> m_ComponentPoolFactory;
        inline static int32_t m_ComponentCount = 0;
    };
}
