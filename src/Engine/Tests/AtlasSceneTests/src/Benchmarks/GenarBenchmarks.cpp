#include <gtest/gtest.h>

#include "AtlasScene/SceneManager.h"
#include "benchmark/benchmark.h"

namespace
{
    struct Transform
    {
        float m_X, m_Y;
    };

    struct Rotation { float m_X, m_Y, m_Z, m_W; };


    atlas::scene::EcsManager g_ecsManager;

    void create100KPairs([[maybe_unused]] const benchmark::State& state)
    {
        g_ecsManager.Clear();

        static bool once = false;
        if (!once)
        {
            once = true;
            atlas::scene::ComponentRegistry::RegisterComponent<Transform>({});
            atlas::scene::ComponentRegistry::RegisterComponent<Rotation>({});
        }

        for(int i = 0; i <  100000; ++i)
        {
            const auto entity = g_ecsManager.AddEntity();
            g_ecsManager.AddComponent<Transform>(entity);
            g_ecsManager.AddComponent<Rotation>(entity);
        }
    }

    void resetEcs([[maybe_unused]] const benchmark::State& state)
    {
        g_ecsManager.Clear();
    }

    void iterationNormal([[maybe_unused]] benchmark::State& state)
    {
        for (auto _ : state)
        {
            for(auto [entity, transform, rotation] : g_ecsManager.IterateEntityComponents<Transform, Rotation>())
            {
                transform.m_X++;
                rotation.m_W++;
            }
        }
    }

    void iterationCopiedIds(benchmark::State& state)
    {
        for (auto _ : state)
        {
            const auto entities = g_ecsManager.GetEntitiesWithComponents<Transform, Rotation>();
            for(const atlas::scene::EntityId entity : entities)
            {
                auto& transform = g_ecsManager.GetComponent<Transform>(entity);
                auto& rotation = g_ecsManager.GetComponent<Rotation>(entity);
                transform.m_X++;
                rotation.m_W++;
            }
        }
    }

    void iterationCopiedIdsTupleReturn([[maybe_unused]] benchmark::State& state)
    {
        for (auto _ : state)
        {
            const auto entities = g_ecsManager.GetEntitiesWithComponents<Transform, Rotation>();
            for(const atlas::scene::EntityId entity : entities)
            {
                auto [transform, rotation] = g_ecsManager.GetComponents<Transform, Rotation>(entity);
                transform.m_X++;
                rotation.m_W++;
            }
        }
    }

    void iterateSeparately([[maybe_unused]] benchmark::State& state)
    {
        for (auto _ : state)
        {
            for(auto [entity, transform] : g_ecsManager.IterateEntityComponents<Transform>())
            {
                transform.m_X++;
            }

            for(auto [entity,  rotation] : g_ecsManager.IterateEntityComponents<Rotation>())
            {
                rotation.m_W++;
            }
        }
    }
}

BENCHMARK(iterationNormal)->Setup(create100KPairs)->Teardown(resetEcs)->Unit(benchmark::kMicrosecond);
BENCHMARK(iterationCopiedIds)->Setup(create100KPairs)->Teardown(resetEcs)->Unit(benchmark::kMicrosecond);
BENCHMARK(iterationCopiedIdsTupleReturn)->Setup(create100KPairs)->Teardown(resetEcs)->Unit(benchmark::kMicrosecond);
BENCHMARK(iterateSeparately)->Setup(create100KPairs)->Teardown(resetEcs)->Unit(benchmark::kMicrosecond);
