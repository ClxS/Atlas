#include "AtlasScenePCH.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

atlas::scene::EcsManager::EcsManager()
{
    m_ArchetypePools.emplace_back(0);
}

atlas::scene::EcsManager::~EcsManager()
{
    for (auto& [_1, _2, components] : m_ArchetypePools)
    {
        for (auto [_, pool] : components)
        {
            delete pool;
        }

        components.clear();
    }

    m_ArchetypePools.clear();
}

atlas::scene::EntityId atlas::scene::EcsManager::AddEntity()
{
    const EntityId id{m_EntityIndices.Size()};
    ArchetypeIndex archetypeIndex = ArchetypeIndex::Empty();
    auto& [_1, pool, _2] = GetPool(archetypeIndex);

    m_EntityIndices.Push(pool.Size(), archetypeIndex);
    pool.Push(id);
    return id;
}

void atlas::scene::EcsManager::RemoveEntity(const EntityId entity)
{
    // TODO: Validate entities are actually extant throughout the manager
    const auto removedIndex = m_EntityIndices.GetCopy(entity.m_Value);
    auto& [_, entityPool, components] = GetPool(removedIndex.m_ArchetypeIndex);

    if (entityPool.Size() > 1)
    {
        const auto endEntityId = entityPool.GetCopy(entityPool.Size() - 1);
        if (endEntityId != entity)
        {
            m_EntityIndices.Remove(entity.m_Value);
            m_EntityIndices.Set(endEntityId.m_Value, removedIndex);
            entityPool.SwapAndPop(removedIndex.m_EntityIndex);
            for (const auto& [poolMask, pool] : components)
            {
                pool->SwapAndPop(removedIndex.m_EntityIndex);
            }
        }
        else
        {
            m_EntityIndices.Remove(entity.m_Value);
            entityPool.Pop();
            for (const auto& [_, pool] : components)
            {
                pool->Pop();
            }
        }
    }
    else
    {
        m_EntityIndices.Remove(entity.m_Value);
        entityPool.Pop();
        for (const auto& [_, pool] : components)
        {
            pool->Pop();
        }
    }
}

void atlas::scene::EcsManager::Clear()
{
    m_EntityIndices.Clear();
    for(auto& pool : m_ArchetypePools)
    {
        pool.Clear();
    }
}

bool atlas::scene::EcsManager::DoesEntityHaveComponent(const EntityId entity,
    const ComponentInfoId componentInfoId) const
{
    const auto [_, oldArchetypeIndex] = m_EntityIndices.GetCopy(entity.m_Value);
    assert(oldArchetypeIndex.m_ArchetypeIndex >= 0);

    const ArchetypePool& pool = GetPool(oldArchetypeIndex);

    const uint64_t targetMask = ComponentRegistry::GetComponentMask(componentInfoId);
    return pool.m_ArchetypeComponentMask & targetMask;
}

void* atlas::scene::EcsManager::GetComponent(const EntityId entityId, const ComponentInfoId componentInfoId)
{
    const auto [entityIndex, oldArchetypeIndex] = m_EntityIndices.GetCopy(entityId.m_Value);
    assert(oldArchetypeIndex.m_ArchetypeIndex >= 0);

    auto& archetypePool = GetPool(oldArchetypeIndex);
    auto& componentPool = archetypePool.GetComponentPool(ComponentRegistry::GetComponentMask(componentInfoId));

    return componentPool.m_Pool->Get(entityIndex);
}

void* atlas::scene::EcsManager::AddComponentCore(const EntityId entity, const ComponentInfoId componentId, std::function<void*(PoolBase*)> setInputEntity)
{
    const auto [oldEntityIndex, oldArchetypeIndex] = m_EntityIndices.GetCopy(entity.m_Value);
    assert(oldEntityIndex >= 0);
    assert(oldArchetypeIndex.m_ArchetypeIndex >= 0);

    ArchetypeIndex newArchetypeIndex = ArchetypeIndex::Empty();
    {
        const uint64_t newMask = m_ArchetypePools[oldArchetypeIndex.m_ArchetypeIndex].m_ArchetypeComponentMask |
            ComponentRegistry::GetComponentMask(componentId);
        newArchetypeIndex = GetOrCreateArchetype(newMask);
    }

    // Important! We need to re-obtain the old pool in case the GetOrCreateArchetype caused an expansion (and thus moved it)
    ArchetypePool& oldPool = GetPool(oldArchetypeIndex);

    auto& [newArchetypeMask, newEntityPool, newComponentPools] = GetPool(newArchetypeIndex);

    m_EntityIndices.Set(entity.m_Value, EntityIndex{newEntityPool.Size(), newArchetypeIndex});

    void* returnValue = nullptr;

    for (auto [poolMask, componentPool] : newComponentPools)
    {
        if (oldPool.m_ArchetypeComponentMask & poolMask)
        {
            componentPool->ClaimFromOtherPool(oldPool.GetComponentPool(poolMask).m_Pool, oldEntityIndex);
        }
        else
        {
            returnValue = setInputEntity(componentPool);
        }
    }

    newEntityPool.Push(entity);
    if (oldPool.m_EntityPool.Size() > 1 && oldEntityIndex != oldPool.m_EntityPool.Size() - 1)
    {
        oldPool.m_EntityPool.SwapAndPop(oldEntityIndex);
        m_EntityIndices.Set(
            oldPool.m_EntityPool.GetCopy(oldEntityIndex).m_Value,
        {
                oldEntityIndex,
                oldArchetypeIndex
            });
    }
    else
    {
        oldPool.m_EntityPool.Pop();
    }

    assert(returnValue);
    return returnValue;
}

atlas::scene::ArchetypeIndex atlas::scene::EcsManager::GetOrCreateArchetype(uint64_t archetypeMask)
{
    for (int i = 0; i < m_ArchetypePools.size(); ++i)
    {
        if (m_ArchetypePools[i].m_ArchetypeComponentMask == archetypeMask)
        {
            return ArchetypeIndex{i};
        }
    }

    m_ArchetypePools.emplace_back(archetypeMask);
    auto& [mask, entityPool, componentPools] = m_ArchetypePools.back();

    while (archetypeMask > 0)
    {
        const uint64_t componentMask = (1ULL << std::countr_zero(archetypeMask));
        archetypeMask &= ~componentMask;

        componentPools.emplace_back(componentMask, ComponentRegistry::GetFactoryForPoolWithMask(componentMask)());
    }

    return ArchetypeIndex{static_cast<int32_t>(m_ArchetypePools.size()) - 1};
}
