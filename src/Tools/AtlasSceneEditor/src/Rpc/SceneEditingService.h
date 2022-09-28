#pragma once

#include "SceneEditing.grpc.pb.h"
#include "AtlasRpc/AsyncRpcService.h"

namespace atlas::scene_editor::rpc
{
    class SceneEditingServiceImpl final : public SceneEditingService::AsyncService, public atlas::rpc::IAsyncEndpoint
    {
    public:
        grpc::Service* GetService() override;
        void GetEndpointFactories(std::vector<std::unique_ptr<atlas::rpc::IAsyncResponderFactory>>& methods) override;

        grpc::Status CreateNewScene(grpc::ServerContext*, const Unit*, Result*) override;
        grpc::Status ClearScene(grpc::ServerContext*, const Unit*, Result*) override;
        grpc::Status CreateEntity(grpc::ServerContext*, const Unit*, Entity*) override;
    };
}
