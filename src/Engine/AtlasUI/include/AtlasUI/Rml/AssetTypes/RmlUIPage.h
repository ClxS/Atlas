#pragma once
#include <filesystem>

#include "AtlasResource/ResourceAsset.h"

namespace atlas::ui::rml::asset_types
{
    class RmlUiPage final : public resource::ResourceAsset
    {
    public:
        RmlUiPage(std::filesystem::path pagePath, std::string data)
            : m_PagePath{std::move(pagePath)}
            , m_Data{std::move(data)}
        {

        }

        [[nodiscard]] const std::filesystem::path& GetPath() const { return m_PagePath; }
        [[nodiscard]] const std::string& GetString() const { return m_Data; }

    protected:
        std::filesystem::path m_PagePath;
        std::string m_Data;
    };
}
