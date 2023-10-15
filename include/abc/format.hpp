#pragma once

#include "abc/debug.hpp"
#include "abc/optional.hpp"
#include "abc/string.hpp"

#include <vector>
#include <type_traits>
#include <sstream>  //ostringstream

namespace abc
{
//////////////////////////////////////////////////////////////////////////

static_assert(std::is_class<string>::value, "string.hpp is required before this header");

//////////////////////////////////////////////////////////////////////////

namespace detail
{
//////////////////////////////////////////////////////////////////////////

template <typename T, bool IS_FUNDAMENTAL_T = std::is_fundamental<T>::value>
struct to_string_impl
{
    static abc::string impl(const T &value)
    {
        ABC_UNUSED(value);
        static_assert(abc::helpers::fail<T>::value, "Missing type specialization for T");
        return abc::string("");
    }
};

template <typename T, bool IS_FUNDAMENTAL_T = std::is_fundamental<T>::value>
struct from_string_impl
{
    static abc::optional<T> impl(const abc::string &str)
    {
        ABC_UNUSED(str);
        static_assert(abc::helpers::fail<T>::value, "Missing type specialization for T");
        return abc::none;
    }
};

//////////////////////////////////////////////////////////////////////////

template <typename T>
struct to_string_impl<T, true>
{
    static abc::string impl(const T &value)
    {
        std::ostringstream ostr;
        ostr << value;
        return ostr.str();
    }
};

template <typename T>
struct from_string_impl<T, true>
{
    static abc::optional<T> impl(const abc::string &str)
    {
        T                  result;
        std::istringstream istr(str);
        istr >> result;
        return result;
    }
};

}  // namespace detail

//////////////////////////////////////////////////////////////////////////

template <typename T>
abc::optional<T> from_string(const abc::string &str)
{
    return detail::from_string_impl<T>::impl(str);
}

template <typename T>
T from_string(const abc::string &str, T defaultValue)
{
    const abc::optional<T> optResult = detail::from_string_impl<T>::impl(str);
    return optResult.value_or(defaultValue);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
string to_string(const T &value)
{
    return detail::to_string_impl<T>::impl(value);
}

// explicit specialization for strings
template <>
inline string to_string(const string &value)
{
    return value;
}
inline string to_string(const char *value) { return string(value); }

//////////////////////////////////////////////////////////////////////////

///@brief: format("{} %s", param1IsVariadic, param2isString);
template <typename TDest = abc::string, class FormatString, typename... Args>
TDest format(const FormatString &i_format, Args &&... args);

template <typename TDest = abc::string>
TDest format();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace detail
{
//////////////////////////////////////////////////////////////////////////

// forward delcare base case, so argument deduction chooses this option when Ts = {}
template <typename T>
void extractParams(std::vector<string> &o_params, T head);

template <typename T, typename... Ts>
void extractParams(std::vector<string> &o_params, T head, Ts... tail)
{
    extractParams(o_params, head);
    extractParams(o_params, tail...);
}
template <typename T>
void extractParams(std::vector<string> &o_params, T head)
{
    o_params.insert(o_params.end(), abc::to_string(head));
}

#if 0 // UNUSED
template <typename T, typename... Args>
static abc::string GetParamFromArgs(size_t index, const T &head, Args... args)
{
    if (index == 0)
    {
        return abc::to_string(head);
    }

    return GetParamFromArgs(index - 1, std::forward<Args>(args)...);
}
template <typename T>
static abc::string GetParamFromArgs(size_t index, const T &head)
{
    if (index > 0)
    {
        ABC_FAIL("not enough arguments: {} more expected", index);
    }
    return abc::to_string(head);
}
#endif//UNUSED

template <class FormatString = abc::string>
struct format_string_adapter
{
    explicit format_string_adapter(const FormatString &fmt) : m_it(fmt.begin()), m_end(fmt.end) {}
    bool is_done() const { return m_it == m_end; }
    char get() const { return *m_it; }
    char get_and_advance() { return *m_it++; }

private:
    using iterator = typename FormatString::iterator;
    iterator m_it;
    iterator m_end;
};
template <>
struct format_string_adapter<char *>
{
    explicit format_string_adapter(char *&fmt) : m_it(fmt) {}
    bool is_done() const { return *(m_it + 1) == 0; }
    char get() const { return *m_it; }
    char get_and_advance() { return *m_it++; }

private:
    const char *m_it;
};
template <size_t N>
struct format_string_adapter<char[N]>
{
    explicit format_string_adapter(const char (&fmt)[N]) : m_it(fmt), m_end(fmt + N - 1) {}
    bool is_done() const { return m_it == m_end; }
    char get() const { return *m_it; }
    char get_and_advance() { return *m_it++; }

private:
    const char *m_it;
    const char *m_end;
};

template <typename TDst = abc::string>
struct dst_adapter
{
    explicit dst_adapter(TDst &dest) : m_buffer(dest.begin(), dest.end()), m_dest(dest) {}

    void append(char c) { m_buffer.push_back(c); }
    void append(char const *start, char const *end)
    {
        m_buffer.append(start, start + (end - start));
    }
    void append(const abc::string &str) { m_buffer.append(str); }
    void clear()
    {
        m_dest.clear();
        m_buffer.clear();
    }
    void finish() { m_dest = m_buffer; }

private:
    abc::string  m_buffer;
    abc::string &m_dest;
};

template <typename TDest>
struct format_helper
{
    template <typename FormatString, typename... Args>
    static TDest format(const FormatString &i_format, Args &&... args)
    {
        auto formatString = format_string_adapter<FormatString>(i_format);
        if (formatString.is_done())
        {
            return i_format;
        }

        TDest              formattedString;
        dst_adapter<TDest> outputAdapter(formattedString);

        std::vector<string> params;
        params.reserve(sizeof...(args));
        detail::extractParams(params, args...);

        auto   paramsIt          = params.begin();
        size_t placeholdersCount = 0;

        while (!formatString.is_done())
        {
            char curChar = formatString.get_and_advance();

            if (curChar == '%')
            {
                ++placeholdersCount;
                while (!formatString.is_done())
                {
                    curChar = formatString.get_and_advance();
                    if (curChar == 's')
                    {
                        const abc::string &param = *paramsIt++;
                        outputAdapter.append(param);
                        break;
                    }
                    else if (curChar == 'd')
                    {
                        const abc::string &param = *paramsIt++;
                        outputAdapter.append(param);
                        break;
                    }
                    else if (curChar == 'f')
                    {
                        const abc::string &param = *paramsIt++;
                        outputAdapter.append(param);
                        break;
                    }
                    else
                    {
                        --placeholdersCount;  // not a placeholder
                        outputAdapter.append('%');
                        outputAdapter.append(curChar);
                        break;
                    }
                }
                if (curChar == '%')
                {
                    --placeholdersCount;  // not a placeholder
                    outputAdapter.append(curChar);
                }
            }
            else if (curChar == '{')
            {
                ++placeholdersCount;

                curChar = formatString.get_and_advance();
                while (curChar != '}' && !formatString.is_done())
                {
                    curChar = formatString.get_and_advance();
                    // skip formatting parameters, for now
                }
                if (curChar == '}')
                {
                    ABC_ASSERT(paramsIt != params.end(), "Missing argument #{} in '{}'",
                               placeholdersCount, i_format);
                    if (paramsIt != params.end())
                    {
                        const abc::string &param = *paramsIt++;
                        outputAdapter.append(param);
                    }
                    else
                    {
                        outputAdapter.append(
                            format("###Missing argument #{} in '{}'", placeholdersCount, i_format));
                    }
                }
                else
                {
                    outputAdapter.append(
                        format("err missing closing '}' at argument #{}", placeholdersCount));
                }
            }
            else
            {
                outputAdapter.append(curChar);
            }
        }

        outputAdapter.finish();

        return formattedString;
    }

    template <class FormatString>
    static TDest format(const FormatString & /* i_format */)
    {
        return TDest();
    }
};
//////////////////////////////////////////////////////////////////////////
}  // namespace detail

//////////////////////////////////////////////////////////////////////////

template <typename TDest, class FormatString, typename... Args>
TDest format(const FormatString &i_format, Args &&... args)
{
    return detail::format_helper<TDest>::format(i_format, std::forward<Args>(args)...);
}

template <typename TDest, typename T>
TDest format(const T &i_value)
{
    return abc::to_string(i_value);
}

template <typename TDest>
TDest format()
{
    TDest dest;
    return dest;
}

//////////////////////////////////////////////////////////////////////////
}  // namespace abc
