#include "AtlasUIPCH.h"
#include "RmlSystemInterface.h"

#include <SDL.h>

double atlas::ui::rml::RmlSystemInterface::GetElapsedTime()
{
    return static_cast<double>(SDL_GetTicks()) / 1000.0;
}
