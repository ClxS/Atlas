#pragma once
#include <memory>

#include "PointerCoord.h"
#include "UserDevice.h"

namespace atlas::input::control_schemes
{
    class MouseControlScheme;
    class KeyboardControlScheme;
}

namespace atlas::input::devices
{
    class Mouse;
    class Keyboard;
}

namespace atlas::input::profiles
{
    class UserDevice_MouseKeyboard final : public UserDevice
    {
    public:
        UserDevice_MouseKeyboard(
            std::unique_ptr<devices::Mouse>&& mouse,
            std::unique_ptr<devices::Keyboard>&& keyboard,
            control_schemes::MouseControlScheme* mouseControlScheme = nullptr,
            control_schemes::KeyboardControlScheme* keyboardControlScheme = nullptr)
            : m_MouseControlScheme{mouseControlScheme}
              , m_KeyboardControlScheme{keyboardControlScheme}
              , m_Mouse{std::move(mouse)}
              , m_Keyboard{std::move(keyboard)}
        {
        }

        [[nodiscard]] bool IsButtonDown(core::StringHashView button) const override;
        [[nodiscard]] bool IsButtonMapped(core::StringHashView button) const override;

        [[nodiscard]] int32_t GetAxisValue(core::StringHashView axis) const override;
        [[nodiscard]] int32_t GetAxisAbsoluteValue(core::StringHashView axis) const override;


        [[nodiscard]] uint8_t GetPointerCount() const override;

        [[nodiscard]] PointerCoord GetPointerPosition(int32_t pointerIndex = 0) const override;
        [[nodiscard]] PointerCoord GetPointerDeltaPosition(int32_t pointerIndex = 0) const override;

        [[nodiscard]] const control_schemes::MouseControlScheme* GetMouseControlScheme() const
        {
            return m_MouseControlScheme;
        }
        void SetMouseControlScheme(control_schemes::MouseControlScheme* scheme)
        {
            m_MouseControlScheme = scheme;
        }

        [[nodiscard]] const control_schemes::KeyboardControlScheme* GetKeyboardControlScheme() const
        {
            return m_KeyboardControlScheme;
        }
        void SetKeyboardControlScheme(control_schemes::KeyboardControlScheme* scheme)
        {
            m_KeyboardControlScheme = scheme;
        }

    private:
        friend class UserInputManager;

        control_schemes::MouseControlScheme* m_MouseControlScheme {nullptr};
        control_schemes::KeyboardControlScheme* m_KeyboardControlScheme {nullptr};
        std::unique_ptr<devices::Mouse> m_Mouse;
        std::unique_ptr<devices::Keyboard> m_Keyboard;
    };
}
