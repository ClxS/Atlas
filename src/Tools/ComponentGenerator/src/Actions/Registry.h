#pragma once
#include "ExitCodes.h"

struct Arguments;

namespace component_generator::actions
{
    ExitCode registry(const Arguments& args);
}
