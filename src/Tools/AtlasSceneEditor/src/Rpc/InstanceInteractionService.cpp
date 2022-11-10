#include "AtlasSceneEditorPCH.h"
#include "InstanceInteractionService.h"

#include "Constants.h"
#include "AtlasScene/ECS/Components/ComponentRegistry.h"
#include "AtlasTrace/Logging.h"

grpc::Service* atlas::scene_editor::rpc::InstanceInteractionServiceImpl::GetService()
{
    return this;
}

void atlas::scene_editor::rpc::InstanceInteractionServiceImpl::GetEndpointFactories(
    std::vector<std::unique_ptr<atlas::rpc::IAsyncResponderFactory>>& methods)
{
    methods.push_back(ATLAS_RPC_BIND_METHOD(GetComponentRegistry, Unit, ComponentRegistry));
    methods.push_back(ATLAS_RPC_BIND_METHOD(RegisterNotificationEndpoint, SceneUpdateRegistration, Result));
    methods.push_back(ATLAS_RPC_BIND_METHOD(GetRenderMasks, Unit, RenderMasksInfo));
}

grpc::Status atlas::scene_editor::rpc::InstanceInteractionServiceImpl::RegisterNotificationEndpoint(
    grpc::ServerContext* serverContext, const SceneUpdateRegistration* sceneUpdateRegistration, Result* result)
{
    AT_ERROR(AtlasRpc, "Not implemented");
    return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, ""};
}

grpc::Status atlas::scene_editor::rpc::InstanceInteractionServiceImpl::GetComponentRegistry(
    grpc::ServerContext* serverContext, const Unit* unit, ComponentRegistry* componentRegistry)
{
    std::lock_guard lock { scene::ComponentRegistry::GetComponentRegistrationMutex() };
    for(auto& component : scene::ComponentRegistry::GetComponentRegistrations())
    {
        ComponentInfo& info = *componentRegistry->add_components();
        info.set_typeid_(component.m_UniqueId.m_Value);
        info.set_componentname(std::string(component.m_ComponentName).c_str());
        for(auto& field : component.m_Fields)
        {
            ComponentFieldInfo& fieldInfo = *info.add_fields();
            fieldInfo.set_fieldname(std::string(field.m_Name));
            fieldInfo.set_type(std::string(field.m_Type));
        }
        for(auto& metadataEntry : component.m_Metadata)
        {
            ComponentMetadataEntry& metadata = *info.add_metadata();
            metadata.set_key(std::string(metadataEntry.m_Key));
            metadata.set_value(std::string(metadataEntry.m_Value));
        }
    }

    return grpc::Status::OK;
}

grpc::Status atlas::scene_editor::rpc::InstanceInteractionServiceImpl::GetRenderMasks(
    grpc::ServerContext* serverContext, const Unit* unit, RenderMasksInfo* renderMasksInfo)
{
    RenderMask* mask = renderMasksInfo->add_masks();
    mask->set_name("General Geometry");
    mask->set_mask(constants::render_masks::c_generalGeometry);

    mask = renderMasksInfo->add_masks();
    mask->set_name("Shader Caster");
    mask->set_mask(constants::render_masks::c_shadowCaster);

    mask = renderMasksInfo->add_masks();
    mask->set_name("Pickable");
    mask->set_mask(constants::render_masks::c_pickable);

    return grpc::Status::OK;
}
