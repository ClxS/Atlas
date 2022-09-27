#pragma once

#include <filesystem>
#include <string>
#include "ToolsCore/ArgumentParser.h"

struct Arguments : public cppconv::tools::arg_parser::ArgumentsBase
{
    ARG_CTOR_4_V(Arguments, Mode, ComponentRoot, ProjectName, Namespace, Groups)

    VERB_2(Mode, "generate", "registry");

    ARG(ComponentRoot, std::filesystem::path, 'r', "root", "(generate, registry) The path to the root component folder");
    ARG(ProjectName, std::string, 'p', "project", "(generate, registry) The name of the project whose components are being generated");
    ARG(Namespace, std::string, 'n', "namespace", "(generate, registry) The namespace which will contain the generated components");

    ARG(Groups, std::string, 'g', "groups", "(registry) Semi-colon delimited list of component namespace groups to generate a registry for");

};
