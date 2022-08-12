#pragma once
#include <filesystem>

#include "AtlasResource/ResourceAsset.h"

namespace atlas::ui::rml::asset_types
{
    class RmlRawDataAsset : public resource::ResourceAsset
    {
    public:
        RmlRawDataAsset(std::filesystem::path pagePath, std::unique_ptr<uint8_t[]> data, size_t dataSize)
            : m_PagePath{std::move(pagePath)}
            , m_Data{std::move(data)}
            , m_DataSize{dataSize}
        {
        }

        [[nodiscard]] const std::filesystem::path& GetPath() const { return m_PagePath; }
        [[nodiscard]] const uint8_t* GetData() const { return m_Data.get(); }
        [[nodiscard]] size_t GetDataSize() const { return m_DataSize; }

    protected:
        std::filesystem::path m_PagePath;
        std::unique_ptr<uint8_t[]> m_Data;
        size_t m_DataSize;
    };
}
