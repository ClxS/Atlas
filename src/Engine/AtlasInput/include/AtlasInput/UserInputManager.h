#pragma once
#include <array>
#include <memory>
#include <vector>

#include "Profiles/UserDevice.h"
#include "AtlasInput/ControlSchemes/KeyboardControlScheme.h"
#include "AtlasInput/ControlSchemes/MouseControlScheme.h"

namespace atlas::input
{
    namespace control_schemes
    {
        class KeyboardControlScheme;
        class MouseControlScheme;
    }

    class UserInputManager
    {
        static constexpr int32_t c_userSlots = 4;

    public:
        ~UserInputManager() = default;
        UserInputManager(const UserInputManager&) = delete;
        UserInputManager(UserInputManager&&) = delete;
        UserInputManager& operator=(const UserInputManager&) = delete;
        UserInputManager& operator=(UserInputManager&&) = delete;

        static UserInputManager& Get()
        {
            static UserInputManager s_manager;
            return s_manager;
        }

        bool Initialise();
        void Tick() const;

        void AddControlScheme(std::unique_ptr<control_schemes::KeyboardControlScheme>&& keyboardControlScheme);
        void AddControlScheme(std::unique_ptr<control_schemes::MouseControlScheme>&& mouseControlScheme);

        [[nodiscard]] int32_t GetUserCount() const { return m_UserCount; }

        [[nodiscard]] const profiles::UserDevice* GetUser(int32_t slot) const;

    private:
        struct UserEntry
        {
            enum class Type
            {
                MouseKeyboard,
            };

            Type m_Type;
            std::unique_ptr<profiles::UserDevice> m_Profile;
        };

        UserInputManager() = default;
        UserEntry CreateMouseKeyboardUser() const;
        void UpdateKeyboards() const;
        void UpdateMice() const;

        std::array<UserEntry, c_userSlots> m_Users{};

        struct
        {
            std::vector<std::unique_ptr<control_schemes::KeyboardControlScheme>> m_Keyboard;
            std::vector<std::unique_ptr<control_schemes::MouseControlScheme>> m_Mouse;
        } m_ControlSchemes;

        int32_t m_UserCount{0};
    };
}
