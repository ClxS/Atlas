#include "Arguments.h"
#include "ExitCodes.h"
#include "Registry.h"
#include "ToolsCore/Hashing.h"

#include "Actions/Generate.h"
#include "AtlasTrace/Logging.h"

ExitCode mainImpl(const int argc, char **argv)
{
    const Arguments args;
    if (!args.TryRead(argc, argv))
    {
        AT_ERROR(AssetBuilder, "Failed to parse arguments");
        return ExitCode::ArgumentParseError;
    }

    using cppconv::tools::hashing::fnv1;
    switch (fnv1(args.m_Mode.m_Value))
    {
        case fnv1("generate"): return component_generator::actions::generate(args);
        case fnv1("registry"): return component_generator::actions::registry(args);
        default:
            AT_ERROR(ComponentGenerator, "Unknown mode: {}", args.m_Mode.m_Value);
            return ExitCode::UnknownMode;
    }
}

int main(const int argc, char **argv)
{
    ExitCode exitCode = mainImpl(argc, argv);
    if (exitCode != ExitCode::Success)
    {
        AT_ERROR(ComponentGenerator, "Exit code: {}", static_cast<int>(exitCode));
    }

    return static_cast<int>(exitCode);
}
