#include "AtlasGamePCH.h"
#include "LightingSystem.h"

#include <format>

#include "DirectionalLightComponent.h"
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
    setUniform(m_Uniforms.m_AmbientColour, ambient.data());

    uint8_t lightIndex = 0;
    for(auto [entity, light] : ecs.IterateEntityComponents<components::lighting::DirectionalLightComponent>())
    {
        if (lightIndex >= c_MaxDirectionalLights)
        {
            break;
        }

        bgfx::setUniform(m_Uniforms.m_LightDirections[lightIndex], light.m_Direction.data());
        bgfx::setUniform(m_Uniforms.m_LightColours[lightIndex], light.m_Colour.GetVector4f().data());
        lightIndex++;
    }
}
