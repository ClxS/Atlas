#pragma once
#include "ExitCodes.h"

struct Arguments;

namespace component_generator::actions
{
    ExitCode generate(const Arguments& args);
}
