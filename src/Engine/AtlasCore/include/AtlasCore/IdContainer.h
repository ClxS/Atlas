#pragma once
#include <cstdint>

namespace atlas::core
{
    template<typename TSelf, typename TValueType = int32_t>
    struct IdContainer
    {
        static constexpr int c_invalidValue = -1;

        IdContainer()
        {
        }

        explicit IdContainer(const TValueType value)
            : m_Value(value)
        {
        }

        static TSelf Invalid() { return TSelf{c_invalidValue}; }

        [[nodiscard]] bool IsInvalid() const { return m_Value == c_invalidValue; }

        bool operator==(const IdContainer<TSelf, TValueType> other) const
        {
            return m_Value == other.m_Value;
        }

        bool operator<(const IdContainer<TSelf, TValueType> other) const
        {
            return m_Value < other.m_Value;
        }

        [[nodiscard]] bool IsValid() const { return m_Value >= 0; }

        TValueType m_Value = c_invalidValue;
    };

#define DEFINE_ID_CONTAINER(TYPE, VALUETYPE)\
    struct TYPE : atlas::core::IdContainer<TYPE, VALUETYPE>\
    {\
        TYPE() {}\
        explicit TYPE(const VALUETYPE value) : IdContainer(value) {}\
    }
}
