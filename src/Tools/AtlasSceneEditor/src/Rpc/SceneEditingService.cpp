#include "AtlasSceneEditorPCH.h"
#include "SceneEditingService.h"

#include "SceneEditorState.h"
#include "AtlasRpc/RpcManager.h"
#include "AtlasTrace/Logging.h"

atlas::scene_editor::rpc::SceneEditingServiceImpl::SceneEditingServiceImpl(SceneEditorState* editorState)
    : m_EditorState{editorState}
{
}

grpc::Service* atlas::scene_editor::rpc::SceneEditingServiceImpl::GetService()
{
    return this;
}

void atlas::scene_editor::rpc::SceneEditingServiceImpl::GetEndpointFactories(
    std::vector<std::unique_ptr<atlas::rpc::IAsyncResponderFactory>>& methods)
{
    methods.push_back(ATLAS_RPC_BIND_METHOD(CreateNewScene, Unit, Result));
    methods.push_back(ATLAS_RPC_BIND_METHOD(ClearScene, Unit, Result));
    methods.push_back(ATLAS_RPC_BIND_METHOD(CreateEntity, Unit, Entity));
}

grpc::Status atlas::scene_editor::rpc::SceneEditingServiceImpl::CreateNewScene(grpc::ServerContext* serverContext,
    const Unit* unit, Result* result)
{
    assert(m_EditorState);
    AT_INFO(AtlasRpc, "ClearScene");

    m_EditorState->ClearScene();

    return grpc::Status::OK;
}

grpc::Status atlas::scene_editor::rpc::SceneEditingServiceImpl::ClearScene(grpc::ServerContext* serverContext,
    const Unit* unit, Result* result)
{
    assert(m_EditorState);
    AT_INFO(AtlasRpc, "ClearScene");

    m_EditorState->ClearScene();

    return grpc::Status::OK;
}

grpc::Status atlas::scene_editor::rpc::SceneEditingServiceImpl::CreateEntity(grpc::ServerContext* serverContext,
    const Unit* unit, Entity* entity)
{
    AT_ERROR(AtlasRpc, "Not implemented");
    return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, ""};
}

grpc::Status atlas::scene_editor::rpc::SceneEditingServiceImpl::DeleteEntity(grpc::ServerContext*, const Entity* entity,
    Result*)
{
    assert(m_EditorState);
    assert(entity);
    AT_INFO(AtlasRpc, "DeleteEntity");

    m_EditorState->DeleteEntity(scene::EntityId { entity->id() });

    return grpc::Status::OK;
}

grpc::Status atlas::scene_editor::rpc::SceneEditingServiceImpl::AddComponent(grpc::ServerContext* serverContext,
    const AddComponentRequest* addComponentRequest, Result* result)
{
    assert(m_EditorState);
    assert(addComponentRequest);
    AT_INFO(AtlasRpc, "AddComponent");

    Entity entity;
    AddComponentsRequest request;
    request.mutable_entity()->CopyFrom(addComponentRequest->entity());
    Component* component = request.add_components();
    component->CopyFrom(addComponentRequest->component());

    return AddComponents(serverContext, &request, result);
}

grpc::Status atlas::scene_editor::rpc::SceneEditingServiceImpl::AddComponents(grpc::ServerContext* serverContext,
    const AddComponentsRequest* addComponentsRequest, Result* result)
{
    assert(m_EditorState);
    assert(addComponentsRequest);
    AT_INFO(AtlasRpc, "AddComponents");

    scene::EcsManager& ecs = m_EditorState->GetEcsManager();
    const scene::EntityId entity { addComponentsRequest->entity().id() };
    for(auto& componentRequest : addComponentsRequest->components())
    {
        scene::ComponentInfoId componentInfoId = scene::ComponentInfoId { componentRequest.type() };

        // Pretty safe to assume sizeof is based off 8-bit sized bytes.
        uint8_t* component;
        if (ecs.DoesEntityHaveComponent(entity, componentInfoId))
        {
            component = static_cast<uint8_t*>(ecs.GetComponent(entity, componentInfoId));
        }
        else
        {
            component = static_cast<uint8_t*>(ecs.AddComponent(entity, componentInfoId));
        }

        const scene::ComponentRegistry::ComponentInfo& componentInfo = scene::ComponentRegistry::GetComponentInfo(componentInfoId);

        for(auto& field : componentRequest.fields())
        {
            const scene::ComponentRegistry::ComponentFieldInfo& fieldInfo = componentInfo.GetFieldInfo(scene::ComponentFieldInfoId { field.fieldid() });
            assert(field.value().size() == fieldInfo.m_Size);

            memcpy(component + fieldInfo.m_Offset, field.value().data(), fieldInfo.m_Size);
        }
    }

    AT_ERROR(AtlasRpc, "Not implemented");
    return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, ""};
}
