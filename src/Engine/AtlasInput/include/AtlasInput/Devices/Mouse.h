#pragma once
#include <array>
#include <cstdint>

#include "MouseButton.h"
#include "AtlasInput/PointerCoord.h"
#include "AtlasInput/Devices/MouseAxis.h"

namespace atlas::input::devices
{
    class Mouse
    {
    public:
        [[nodiscard]] PointerCoord GetPosition() const
        {
            return m_CurrentCoord;
        }
        [[nodiscard]] PointerCoord GetDeltaPosition() const
        {
            return m_DeltaCoord;
        }
        [[nodiscard]] int32_t GetScrollWheelDelta() const
        {
            return m_ScrollDelta;
        }

        [[nodiscard]] bool IsButtonDown(MouseButton button) const
        {
            return m_ButtonState[static_cast<int>(button)];
        }

        [[nodiscard]] int32_t GetAxis(const MouseAxis axis) const
        {
            switch(axis)
            {
            case MouseAxis::X:
                return m_DeltaCoord.m_X;
            case MouseAxis::Y:
                return m_DeltaCoord.m_Y;
            case MouseAxis::Wheel:
                return m_ScrollDelta;
            }

            return 0;
        }

        [[nodiscard]] int32_t GetAxisAbsolute(const MouseAxis axis) const
        {
            switch(axis)
            {
            case MouseAxis::X:
                return m_CurrentCoord.m_X;
            case MouseAxis::Y:
                return m_CurrentCoord.m_Y;
            case MouseAxis::Wheel:
                return m_ScrollDelta;
            }

            return 0;
        }

    private:
        friend class UserInputManager;

        std::array<bool, static_cast<size_t>(MouseButton::ButtonCount)> m_ButtonState{};

        PointerCoord m_CurrentCoord;
        PointerCoord m_DeltaCoord;

        int32_t m_ScrollDelta;
    };
}
