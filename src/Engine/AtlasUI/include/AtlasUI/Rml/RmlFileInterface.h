#pragma once
#include "AtlasResource/AssetPtr.h"
#include "RmlUi/Core/FileInterface.h"

namespace atlas::ui::rml
{
    namespace asset_types
    {
        class RmlRawDataAsset;
    }

    class RmlFileInterface final : public Rml::FileInterface
    {
    public:
        Rml::FileHandle Open(const Rml::String& path) override;

        void Close(Rml::FileHandle file) override;

        size_t Read(void* buffer, size_t size, Rml::FileHandle file)  override;

        bool Seek(Rml::FileHandle file, long offset, int origin)  override;

        size_t Tell(Rml::FileHandle file)  override;

    private:
        Rml::FileHandle m_IncrementingHandle{};

        struct DataReference
        {
            resource::AssetPtr<asset_types::RmlRawDataAsset> m_File;
            size_t m_CurrentPosition{};
        };

        std::unordered_map<Rml::FileHandle, DataReference> m_OpenAssets;

        static_assert(sizeof(size_t) <= sizeof(Rml::FileHandle));
    };
}
