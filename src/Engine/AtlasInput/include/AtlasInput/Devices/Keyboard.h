#pragma once
#include <array>
#include <cassert>

#include "KeyboardScanCodes.h"

namespace atlas::input::devices
{
    class Keyboard
    {
    public:
        [[nodiscard]] bool IsKeyDown(const keys::Enum key) const
        {
            assert(key < keys::CodeCount);
            return m_Keys[static_cast<int>(key)];
        }

    private:
        friend class UserInputManager;

        std::array<bool, keys::CodeCount> m_Keys{};
    };
}
