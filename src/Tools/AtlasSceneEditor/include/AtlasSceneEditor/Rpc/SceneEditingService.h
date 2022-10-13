#pragma once

#include "SceneEditing.grpc.pb.h"
#include "AtlasRpc/AsyncRpcService.h"

namespace atlas
{
    namespace scene_editor
    {
        class SceneEditorState;
    }
}

namespace atlas::scene_editor::rpc
{
    class SceneEditingServiceImpl final : public SceneEditingService::AsyncService, public atlas::rpc::IAsyncEndpoint
    {
    public:
        SceneEditingServiceImpl(SceneEditorState* editorState);
        grpc::Service* GetService() override;
        void GetEndpointFactories(std::vector<std::unique_ptr<atlas::rpc::IAsyncResponderFactory>>& methods) override;

        grpc::Status CreateNewScene(grpc::ServerContext*, const Unit*, Result*) override;
        grpc::Status ClearScene(grpc::ServerContext*, const Unit*, Result*) override;
        grpc::Status CreateEntity(grpc::ServerContext*, const Unit*, Entity*) override;
        grpc::Status DeleteEntity(grpc::ServerContext*, const Entity*, Result*) override;

        grpc::Status AddComponent(grpc::ServerContext*, const AddComponentRequest*, Result*) override;
        grpc::Status AddComponents(grpc::ServerContext*, const AddComponentsRequest*, Result*) override;


    private:
        SceneEditorState* m_EditorState{nullptr};
    };
}
