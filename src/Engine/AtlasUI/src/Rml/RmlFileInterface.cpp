#include "AtlasUIPCH.h"
#include "RmlFileInterface.h"

#include "AssetTypes/RmlRawDataAsset.h"
#include "AtlasResource/ResourceLoader.h"

Rml::FileHandle atlas::ui::rml::RmlFileInterface::Open(const Rml::String& path)
{
    assert(false); // "Do this"
    return {};
    /*if (path == "rml.rcss")
    {
        const auto asset = atlas::resource::ResourceLoader::LoadAsset<
            cpp_conv::resources::registry::CoreBundle, asset_types::RmlRawDataAsset>(
            cpp_conv::resources::registry::core_bundle::ui::rcss::c_rml);
        if (!asset)
        {
            return {};
        }

        const auto handle = ++m_IncrementingHandle;
        m_OpenAssets[handle] =
        {
            asset
        };

        return handle;
    }

    return {};*/
}

void atlas::ui::rml::RmlFileInterface::Close(const Rml::FileHandle file)
{
    const auto fileIt = m_OpenAssets.find(file);
    if (fileIt != m_OpenAssets.end())
    {
        m_OpenAssets.erase(fileIt);
    }
}

size_t atlas::ui::rml::RmlFileInterface::Read(void* buffer, const size_t size, const Rml::FileHandle file)
{
    const auto fileIter = m_OpenAssets.find(file);
    if (fileIter == m_OpenAssets.end())
    {
        assert(false);
        return 0;
    }

    auto& fileRef = fileIter->second;

    const auto remainingSpace = fileRef.m_File->GetDataSize() - fileRef.m_CurrentPosition;
    const auto toRead = std::min(remainingSpace, size);

    std::memcpy(buffer, fileRef.m_File->GetData(), toRead);

    fileRef.m_CurrentPosition += toRead;
    return toRead;
}

bool atlas::ui::rml::RmlFileInterface::Seek(const Rml::FileHandle file, const long offset, const int origin)
{
    const auto fileIter = m_OpenAssets.find(file);
    if (fileIter == m_OpenAssets.end())
    {
        assert(false);
        return false;
    }

    const auto size = fileIter->second.m_File->GetDataSize();

    //origin One of either SEEK_SET (seek from the beginning of the file), SEEK_END (seek from the end of the file) or SEEK_CUR (seek from the current file position).
    int64_t targetPosition = 0;
    switch (origin)
    {
    case SEEK_SET:
        targetPosition = offset;
        break;
    case SEEK_END:
        targetPosition = static_cast<int64_t>(size) - offset;
        break;
    case SEEK_CUR:
        targetPosition = static_cast<int64_t>(fileIter->second.m_CurrentPosition) + offset;
        break;
    default:
        assert(false);
        break;
    }

    if (targetPosition < 0 || targetPosition > static_cast<int64_t>(size))
    {
        return false;
    }

    fileIter->second.m_CurrentPosition = targetPosition;
    return true;
}

size_t atlas::ui::rml::RmlFileInterface::Tell(const Rml::FileHandle file)
{
    return m_OpenAssets[file].m_CurrentPosition;
}
