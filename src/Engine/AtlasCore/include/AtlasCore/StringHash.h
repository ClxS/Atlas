#pragma once
#include <cstdint>
#include <string>
#include "AtlasCore/Hashing.h"

namespace atlas::core
{
    using StringId = uint64_t;

    struct StringHashView
    {
        StringId m_StringHash;
        std::string_view m_String;

        explicit StringHashView(const std::string_view value)
            : m_StringHash{hashing::fnv1(value)}
            , m_String{value}
        {
        }

        // ReSharper disable once CppNonExplicitConvertingConstructor
        StringHashView(const struct StringHash& value);
    };

    struct StringHash
    {
        StringId m_StringHash;
        std::string m_String;

        StringHash()
            : m_StringHash{0}
        {
        }

        explicit constexpr StringHash(std::string value)
            : m_StringHash{hashing::fnv1(value)}
            , m_String{std::move(value)}
        {
        }

        explicit StringHash(const StringHashView value)
            : m_StringHash{value.m_StringHash}
            , m_String{value.m_String}
        {
        }
    };

    namespace comparators
    {
        struct StringEqual
        {
            // ReSharper disable once CppInconsistentNaming
            using is_transparent = std::true_type;

            bool operator()(const core::StringHashView l, const core::StringHashView r) const
            {
                return l.m_StringHash == r.m_StringHash && l.m_String == r.m_String;
            }
        };
        struct StringHash
        {
            // ReSharper disable once CppInconsistentNaming
            using is_transparent = std::true_type;

            auto operator()(core::StringHashView l) const noexcept
            {
                return l.m_StringHash;
            }
        };
    }
}

constexpr bool operator==(const atlas::core::StringHash& a, const atlas::core::StringHash& b)
{
    return a.m_StringHash == b.m_StringHash && a.m_String == b.m_String;
}

constexpr bool operator==(const atlas::core::StringHashView& a, const atlas::core::StringHashView& b)
{
    return a.m_StringHash == b.m_StringHash && a.m_String == b.m_String;
}

constexpr bool operator!=(const atlas::core::StringHash& a, const atlas::core::StringHash& b)
{
    return a.m_StringHash != b.m_StringHash || a.m_String != b.m_String;
}

constexpr bool operator==(const atlas::core::StringHash& a, const atlas::core::StringHashView& b)
{
    return a.m_StringHash == b.m_StringHash && a.m_String == b.m_String;
}

constexpr bool operator!=(const atlas::core::StringHash& a, const atlas::core::StringHashView& b)
{
    return a.m_StringHash != b.m_StringHash || a.m_String != b.m_String;
}

constexpr bool operator<(const atlas::core::StringHash& a, const atlas::core::StringHash& b)
{
    return a.m_StringHash < b.m_StringHash;
}

constexpr bool operator<(const atlas::core::StringHash& a, const atlas::core::StringHashView& b)
{
    return a.m_StringHash < b.m_StringHash;
}

constexpr bool operator<(const atlas::core::StringHashView& a, const atlas::core::StringHashView& b)
{
    return a.m_StringHash < b.m_StringHash;
}
