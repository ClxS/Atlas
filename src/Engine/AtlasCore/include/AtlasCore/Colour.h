#pragma once
#include <cstdint>
#include <Eigen/Core>

namespace atlas::core
{
    class Colour32
    {
    public:
        Colour32(const uint8_t a, const uint8_t r, const uint8_t g, const uint8_t b)
            : m_R(r)
            , m_G(g)
            , m_B(b)
            , m_A(a)
        {
        }

        constexpr explicit Colour32(const uint32_t colour)
            : m_R((colour & 0xFF0000) >> 16)
            , m_G((colour & 0xFF00) >> 8)
            , m_B(colour & 0xFF)
            , m_A((colour & 0xFF000000) >> 24)
        {
        }

        [[nodiscard]] uint32_t GetArgb() const
        {
            return
                static_cast<uint32_t>(m_A) << 24 |
                static_cast<uint32_t>(m_R) << 16 |
                static_cast<uint32_t>(m_G) << 8 |
                static_cast<uint32_t>(m_B) << 0;
        }

        [[nodiscard]] uint32_t GetAbgr() const
        {
            return
                static_cast<uint32_t>(m_A) << 24 |
                static_cast<uint32_t>(m_B) << 16 |
                static_cast<uint32_t>(m_G) << 8 |
                static_cast<uint32_t>(m_R) << 0;
        }

        [[nodiscard]] Eigen::Vector4f GetVector4f() const
        {
            return {
                static_cast<float>(m_R) / 255.0f,
                static_cast<float>(m_G) / 255.0f,
                static_cast<float>(m_B) / 255.0f,
                static_cast<float>(m_A) / 255.0f,
            };
        }

        [[nodiscard]] Colour32 operator*(const Eigen::Vector4f& other) const
        {
            return Colour32 {
                static_cast<uint8_t>(static_cast<float>(m_A) * other.w()),
                static_cast<uint8_t>(static_cast<float>(m_R) * other.x()),
                static_cast<uint8_t>(static_cast<float>(m_G) * other.y()),
                static_cast<uint8_t>(static_cast<float>(m_B) * other.z()),
            };
        }

        [[nodiscard]] Colour32 operator*(const Colour32& other) const
        {
            Eigen::Vector4f otherAsVector = other.GetVector4f();
            return Colour32 {
                static_cast<uint8_t>(static_cast<float>(m_A) * otherAsVector.w()),
                static_cast<uint8_t>(static_cast<float>(m_R) * otherAsVector.x()),
                static_cast<uint8_t>(static_cast<float>(m_G) * otherAsVector.y()),
                static_cast<uint8_t>(static_cast<float>(m_B) * otherAsVector.z()),
            };
        }

    private:
        uint8_t m_R = 0x00;
        uint8_t m_G = 0x00;
        uint8_t m_B = 0x00;
        uint8_t m_A = 0xFF;
    };
}

inline atlas::core::Colour32 operator ""_argb(const unsigned long long int value)
{
    return atlas::core::Colour32 { static_cast<uint32_t>(value) };
}

namespace atlas::core::colours
{
    constexpr Colour32 c_black { 0xFF000000 };
    constexpr Colour32 c_white { 0xFFFFFFFF };
    constexpr Colour32 c_red { 0xFFFF0000 };
    constexpr Colour32 c_green { 0xFF00FF00 };
    constexpr Colour32 c_blue { 0xFF0000FF };
}
