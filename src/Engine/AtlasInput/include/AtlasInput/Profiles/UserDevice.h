#pragma once

#include <cstdint>

#include "AtlasCore/StringHash.h"
#include "AtlasInput/PointerCoord.h"

namespace atlas::input::profiles
{
    class UserDevice
    {
    public:
        virtual ~UserDevice() = default;

        [[nodiscard]] virtual int32_t GetAxisValue(core::StringHashView axis) const = 0;
        [[nodiscard]] virtual int32_t GetAxisAbsoluteValue(core::StringHashView axis) const = 0;
        [[nodiscard]] virtual bool IsButtonDown(core::StringHashView button) const = 0;
        [[nodiscard]] virtual bool IsButtonMapped(core::StringHashView button) const = 0;

        [[nodiscard]] virtual uint8_t GetPointerCount() const = 0;
        [[nodiscard]] virtual PointerCoord GetPointerPosition(int32_t pointerIndex = 0) const = 0;
        [[nodiscard]] virtual PointerCoord GetPointerDeltaPosition(int32_t pointerIndex = 0) const = 0;
    };
}
