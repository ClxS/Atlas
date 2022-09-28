#include "AtlasSceneEditorPCH.h"
#include "SceneEditingService.h"

#include "AtlasRpc/RpcManager.h"
#include "AtlasTrace/Logging.h"

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
    AT_ERROR(AtlasRpc, "Not implemented");
    return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, ""};
}

grpc::Status atlas::scene_editor::rpc::SceneEditingServiceImpl::ClearScene(grpc::ServerContext* serverContext,
    const Unit* unit, Result* result)
{
    AT_ERROR(AtlasRpc, "Not implemented");
    return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, ""};
}

grpc::Status atlas::scene_editor::rpc::SceneEditingServiceImpl::CreateEntity(grpc::ServerContext* serverContext,
    const Unit* unit, Entity* entity)
{
    AT_ERROR(AtlasRpc, "Not implemented");
    return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, ""};
}
