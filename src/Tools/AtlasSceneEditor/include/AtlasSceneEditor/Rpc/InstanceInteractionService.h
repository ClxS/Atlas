#pragma once

#include "InstanceInteraction.grpc.pb.h"
#include "AtlasRpc/AsyncRpcService.h"
#include "AtlasRpc/RpcManager.h"

namespace atlas::scene_editor::rpc
{
    class InstanceInteractionServiceImpl final : public InstanceInteractionService::AsyncService, public atlas::rpc::IAsyncEndpoint
    {
    public:
        grpc::Service* GetService() override;

        void GetEndpointFactories(std::vector<std::unique_ptr<atlas::rpc::IAsyncResponderFactory>>& methods) override;

        grpc::Status RegisterNotificationEndpoint(grpc::ServerContext*, const SceneUpdateRegistration*, Result*) override;
        grpc::Status GetComponentRegistry(grpc::ServerContext*, const Unit*, ComponentRegistry*) override;
    };
}
