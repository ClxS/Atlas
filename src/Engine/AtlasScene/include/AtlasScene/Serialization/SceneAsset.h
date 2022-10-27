#pragma once
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceAsset.h"

namespace atlas::scene
{
    class SceneAsset
    {
    public:

    };

    resource::AssetPtr<resource::ResourceAsset> sceneLoadHandler(const resource::FileData& data);
}
