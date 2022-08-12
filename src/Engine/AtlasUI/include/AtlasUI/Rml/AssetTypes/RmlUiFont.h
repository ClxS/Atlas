#pragma once
#include "RmlRawDataAsset.h"

namespace atlas::ui::rml::asset_types
{
    class RmlUiFont final : public RmlRawDataAsset
    {
    public:
        RmlUiFont(std::filesystem::path pagePath, std::unique_ptr<uint8_t[]> data, const size_t dataSize)
            : RmlRawDataAsset(std::move(pagePath), std::move(data), dataSize)
        {}
    };
}
