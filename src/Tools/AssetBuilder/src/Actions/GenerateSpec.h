#pragma once
#include <filesystem>
#include <string>

#include "ExitCodes.h"

struct Arguments;

namespace asset_builder::actions
{
    std::string getAssetRelativeName(std::string_view outputGroup, std::filesystem::path relativePath);

    ExitCode generateSpec(const Arguments& args);
}
