#include "AtlasSceneEditorPCH.h"
#include "InstanceInteractionService.h"
#include "AtlasScene/ECS/Components/ComponentRegistry.h"

grpc::Service* atlas::scene_editor::rpc::InstanceInteractionServiceImpl::GetService()
{
    return this;
}

void atlas::scene_editor::rpc::InstanceInteractionServiceImpl::GetEndpointFactories(
    std::vector<std::unique_ptr<atlas::rpc::IAsyncResponderFactory>>& methods)
{
    methods.push_back(ATLAS_RPC_BIND_METHOD(GetComponentRegistry, Unit, ComponentRegistry));
    methods.push_back(ATLAS_RPC_BIND_METHOD(RegisterNotificationEndpoint, SceneUpdateRegistration, Result));
}

grpc::Status atlas::scene_editor::rpc::InstanceInteractionServiceImpl::RegisterNotificationEndpoint(
    grpc::ServerContext* serverContext, const SceneUpdateRegistration* sceneUpdateRegistration, Result* result)
{
    return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, ""};
}

grpc::Status atlas::scene_editor::rpc::InstanceInteractionServiceImpl::GetComponentRegistry(
    grpc::ServerContext* serverContext, const Unit* unit, ComponentRegistry* componentRegistry)
{
    std::lock_guard lock { scene::ComponentRegistry::GetComponentRegistrationMutex() };
    for(auto& component : scene::ComponentRegistry::GetComponentRegistrations())
    {
        ComponentInformation& info = *componentRegistry->add_components();
        info.set_typeid_(component.m_UniqueId);
        info.set_componentname(std::string(component.m_ComponentName).c_str());
        for(auto& field : component.m_Fields)
        {
            ComponentField& fieldInfo = *info.add_fields();
            fieldInfo.set_fieldname(std::string(field.m_Name));
            fieldInfo.set_type(std::string(field.m_Type));
        }
    }

    return grpc::Status::OK;
}
