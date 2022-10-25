#pragma once
#include <memory>

namespace atlas::input::profiles
{
    class UserDevice;

    class UserDeviceProfile
    {
    public:
        [[nodiscard]] const UserDevice* GetDevice() const
        {
            return m_Device.get();
        }

    private:
        std::unique_ptr<UserDevice> m_Device;
    };
}
