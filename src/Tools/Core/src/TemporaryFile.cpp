#include "Utility/TemporaryFile.h"

#include <filesystem>
#include <mutex>

static std::mutex s_fileCreationMutex;

atlas::tools::utility::TemporaryFile::TemporaryFile()
{
    std::lock_guard<std::mutex> lock(s_fileCreationMutex);
    m_Path = std::tmpnam(nullptr);
}

atlas::tools::utility::TemporaryFile::TemporaryFile(const std::string_view extension)
{
    std::lock_guard<std::mutex> lock(s_fileCreationMutex);
    m_Path = std::tmpnam(nullptr);
    m_Path.replace_extension(extension);
}

atlas::tools::utility::TemporaryFile::~TemporaryFile()
{
    std::filesystem::remove(m_Path);
}
