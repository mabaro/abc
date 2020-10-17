#pragma once
#include <optional>

namespace abc
{
    //////////////////////////////////////////////////////////////////////////

#if 1

    template <typename T>
    using optional = std::optional<T>;
    using none_t = std::nullopt_t;
    inline constexpr none_t none = std::nullopt;

#else

    static struct none_t
    {
    } none;

    template <typename T>
    class optional
    {
        using this_t = optional<T>;
        using value_t = T;
        using raw_type = typename std::remove_reference<T>::type;
        using ptr_t = raw_type *;
        using const_ptr_t = const ptr_t;
        using ref_t = raw_type &;
        using const_ref_t = const ref_t;
        using rvalue_t = raw_type &&;
        using const_rvalue_t = const raw_type &&;
        using rvalue_t = raw_type &&;

        raw_type *m_value = nullptr;

    public:
        optional() {}
        explicit optional(abc::none_t) {}
        optional(const_ref_t &value) : m_value(new raw_type(value)) {}
        optional(rvalue_t value) : m_value(new raw_type(std::move(value))) {}

        template <typename... Args>
        ref_t emplace(Args &&... args)
        {
            delete m_value;
            m_value = new T(std::forward(args...));
            return *m_value;
        }

        this_t &operator=(abc::none_t)
        {
            delete m_value;
            m_value = nullptr;

            return *this;
        }
        this_t &operator=(const_ref_t &value)
        {
            if (!m_value)
            {
                m_value = new T;
            }
            *m_value = value;
            return *this;
        }

        ptr_t operator->() { return m_value; }
        const_ptr_t operator->() const { return m_value; }
        ref_t operator*() { return *m_value; }
        const_ref_t operator*() const { return *m_value; }
        rvalue_t operator*() { return std::move(*m_value); }
        const_rvalue_t operator*() const { return std::move(*m_value); }

        bool has_value() const { return m_value != nullptr; }
        value_t value_or(const T &defaultValue) const { return m_value ? *m_value : defaultValue; }

        ref_t value() { return *m_value; }
        const_ref_t value() const { return *m_value; }
        rvalue_t value() { return std::move(*m_value); }
        const_rvalue_t value() const { return std::move(*m_value); }

        value_t extract()
        {
            T tmp = std::move(*m_value);
            m_value = nullptr;
            return std::move(tmp);
        }
        void reset() { m_value = nullptr; }

        void swap(this_t &other) noexcept { std::swap(m_value, other.m_value); }
    };

    template <typename T>
    bool operator==(const optional<T> &a, const T &b)
    {
        return a.has_value() && a.get() == b;
    }
    template <typename T>
    bool operator!=(const optional<T> &a, const T &b)
    {
        return !(a == b);
    }
    template <typename T>
    bool operator<(const optional<T> &a, const T &b)
    {
        return !a.has_value() || a.get() < b;
    }
    template <typename T>
    bool operator>(const optional<T> &a, const T &b)
    {
        return a.has_value() && a.get() > b;
    }
    template <typename T>
    bool operator<=(const optional<T> &a, const T &b)
    {
        return !a.has_value() || a.get() <= b;
    }
    template <typename T>
    bool operator>=(const optional<T> &a, const T &b)
    {
        return a.has_value() && a.get() >= b;
    }

    template <typename T>
    bool operator==(const optional<T> &a, const optional<T> &b)
    {
        return a.has_value() && b.has_value() && a.get() == b;
    }
    template <typename T>
    bool operator!=(const optional<T> &a, const optional<T> &b)
    {
        return !a.has_value() || !b.has_value() || a.get() != b;
    }
    template <typename T>
    bool operator<(const optional<T> &a, const optional<T> &b)
    {
        return (!a.has_value() && b.has_value()) || (a.has_value() && b.has_value() && a.get() < b);
    }
    template <typename T>
    bool operator>(const optional<T> &a, const optional<T> &b)
    {
        return (a.has_value() && !b.has_value()) || (a.has_value() && b.has_value() && a.get() > b);
    }
    template <typename T>
    bool operator<=(const optional<T> &a, const optional<T> &b)
    {
        return (!a.has_value() && b.has_value()) || (a.has_value() && b.has_value() && a.get() <= b);
    }
    template <typename T>
    bool operator>=(const optional<T> &a, const optional<T> &b)
    {
        return (a.has_value() && !b.has_value()) || (a.has_value() && b.has_value() && a.get() >= b);
    }

#endif

    //////////////////////////////////////////////////////////////////////////
} // namespace abc
