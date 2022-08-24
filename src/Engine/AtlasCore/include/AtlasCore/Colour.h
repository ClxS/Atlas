#pragma once
#include <cstdint>

namespace atlas::core
{
    enum class Colours : uint32_t
    {
        Black = 0xFF000000,
        White = 0xFFFFFFFF,
    };

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

        explicit Colour32(const uint32_t colour)
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
