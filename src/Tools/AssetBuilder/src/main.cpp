#include <iostream>

#include "Arguments.h"
#include "ExitCodes.h"
#include "ToolsCore/Hashing.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "Actions/Cook.h"
#include "Actions/GenerateSpec.h"
#include "AtlasTrace/Logging.h"
#include "Processing/AssetProcessor.h"

ExitCode mainImpl(const int argc, char **argv)
{
    const Arguments args;
    if (!args.TryRead(argc, argv))
    {
        AT_ERROR(AssetBuilder, "Failed to parse arguments");
        return ExitCode::ArgumentParseError;
    }

    AssetProcessor::initDefaults();

    using cppconv::tools::hashing::fnv1;
    switch (fnv1(args.m_Mode.m_Value))
    {
        case fnv1("generate-spec"): return asset_builder::actions::generateSpec(args);
        case fnv1("cook"): return asset_builder::actions::cook(args);
        default:
            AT_ERROR(AssetBuilder, "Unknown mode: {}", args.m_Mode.m_Value);
            return ExitCode::UnknownMode;
    }
}

int main(const int argc, char **argv)
{
    ExitCode exitCode = mainImpl(argc, argv);
    if (exitCode != ExitCode::Success)
    {
        AT_ERROR(AssetBuilder, "Exit code: {}", static_cast<int>(exitCode));
    }

    return static_cast<int>(exitCode);
}
