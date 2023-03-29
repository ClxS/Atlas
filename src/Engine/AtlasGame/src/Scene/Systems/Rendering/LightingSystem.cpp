#include "AtlasGamePCH.h"
#include "LightingSystem.h"

#include <format>

#include "DirectionalLightComponent.h"
#include "AtlasRender/Debug/DebugDraw.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

void atlas::game::scene::systems::rendering::LightingSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    for(int i = 0; i < c_MaxDirectionalLights; i++)
    {
        auto directionUniformName = std::format("u_lightDirection{}", i);
        auto colourUniformName = std::format("u_lightColour{}", i);
        m_Uniforms.m_LightDirections[i] = createUniform(directionUniformName.c_str(), bgfx::UniformType::Vec4);
        m_Uniforms.m_LightColours[i] = createUniform(colourUniformName.c_str(), bgfx::UniformType::Vec4);
    }

    m_Uniforms.m_AmbientColour = createUniform("u_ambientColour", bgfx::UniformType::Vec4);
}

void atlas::game::scene::systems::rendering::LightingSystem::Render(atlas::scene::EcsManager& ecs)
{
    Eigen::Vector4f ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    setUniform(m_Uniforms.m_AmbientColour.Get(), ambient.data());

    uint8_t lightIndex = 0;
    for(auto [entity, light] : ecs.IterateEntityComponents<components::lighting::DirectionalLightComponent>())
    {
        if (lightIndex >= c_MaxDirectionalLights)
        {
            break;
        }

        setUniform(m_Uniforms.m_LightDirections[lightIndex].Get(), light.m_Direction.data());
        setUniform(m_Uniforms.m_LightColours[lightIndex].Get(), light.m_Colour.GetVector4f().data());
        lightIndex++;

        render::debug::debug_draw::setColor(core::colours::c_red);
        render::debug::debug_draw::drawCylinder(
            { 0.0f, 0.0f, 0.0f },
            { light.m_Direction.x(), light.m_Direction.y(), light.m_Direction.z() },
            0.01f);
    }
}
