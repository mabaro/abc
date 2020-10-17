#pragma once

#include <iostream>
#include <type_traits>

namespace abc
{
    
template <typename T, typename TTag, int64_t DefaultValue>
class tagged_type
{
    static_assert(std::is_trivial_v<T>, "Only trivial types are supported");
    T m_value;

public:
    using value_t = T;
    using this_t  = tagged_type<T, TTag, DefaultValue>;

    static constexpr T default_value = DefaultValue;

public:
    tagged_type(uninitialized_t) {}
    tagged_type() : m_value(default_value) {}
    explicit tagged_type(const T i_value) : m_value(i_value) {}
    tagged_type(const this_t& i_other) : m_value(i_other.m_value) {}
    tagged_type(this_t&& i_other) : m_value(std::move(i_other.m_value)) {}
    ~tagged_type() = default;

    this_t& operator=(const this_t& i_other) noexcept
    {
        m_value = i_other.m_value;
        return *this;
    }
    this_t& operator=(this_t&& i_other) noexcept
    {
        m_value = std::move(i_other.m_value);
        return *this;
    }

    explicit constexpr operator T&() noexcept { return m_value; }
    explicit constexpr operator const T&() const noexcept { return m_value; }

    constexpr const T& value() const noexcept { return m_value; }
    T&                 value() noexcept { return m_value; }

    bool operator==(const this_t& other) const noexcept { return m_value == other.m_value; }
    bool operator!=(const this_t& other) const noexcept { return !operator==(other); }

    using enable_for_arithmetic = std::enable_if<std::is_arithmetic<T>::value, this_t>;

    typename enable_for_arithmetic::type operator+(const this_t& other) const
    {
        return this_t(m_value + other.m_value);
    }
    typename enable_for_arithmetic::type operator-(const this_t& other) const
    {
        return this_t(m_value - other.m_value);
    }
    typename enable_for_arithmetic::type operator*(const this_t& other) const
    {
        return this_t(m_value * other.m_value);
    }
    typename enable_for_arithmetic::type operator/(const this_t& other) const
    {
        return this_t(m_value / other.m_value);
    }

    typename enable_for_arithmetic::type& operator+=(const this_t& other)
    {
        m_value += other.m_value;
        return *this;
    }
    typename enable_for_arithmetic::type& operator-=(const this_t& other)
    {
        m_value -= other.m_value;
        return *this;
    }
    typename enable_for_arithmetic::type& operator*=(const this_t& other)
    {
        m_value *= other.m_value;
        return *this;
    }
    typename enable_for_arithmetic::type& operator/=(const this_t& other)
    {
        m_value /= other.m_value;
        return *this;
    }
};

}  // namespace abc

//#define ABC_NAMED_TYPE_INTEGRAL(NAME, TYPE, DEFAULT_VALUE)     \
//struct ##NAME_traits                                           \
//{                                                              \
//    static const TYPE default_value;                           \
//};                                                             \
//const TYPE ##NAME_traits::default_value = DEFAULT_VALUE;       \
//using NAME = abc::tagged_type<TYPE, struct ##NAME_tag, ##NAME_traits>;
