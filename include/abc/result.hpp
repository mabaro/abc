#pragma once

#include "abc/string.hpp"
#include "abc/optional.hpp"

namespace abc
{
//////////////////////////////////////////////////////////////////////////

namespace detail
{
//////////////////////////////////////////////////////////////////////////
template <typename... TInnerErrors>
class inner_error;

template <typename TError, typename... TInnerErrors>
class inner_error<TError, TInnerErrors...> : protected inner_error<TInnerErrors...>
{
    abc::optional<TError> m_optError;

public:
    inner_error() = default;
    inline inner_error(const inner_error<TError, TInnerErrors...> &other)
        : inner_error<TInnerErrors...>(other)
        , m_optError(other.m_optError)
    {
    }
    inline inner_error(const TError &err)
		: inner_error<TInnerErrors...>()
        , m_optError(err) {}
    inline inner_error(const typename TError::code_t &errorCode)
		: inner_error<TInnerErrors...>()
		, m_optError(errorCode) {}

    template <typename InnerError>
    inline inner_error(InnerError err)
        : inner_error<TInnerErrors...>(err)
    {
    }

    using inner_error<TInnerErrors...>::get;

    template <typename TErrorOrInner = TError>
    inline const abc::optional<TErrorOrInner> &get() const
    {
        return m_optError;
    }
    inline const abc::string message() const
    {
        return m_optError.has_value() ? m_optError->message()
                                      : inner_error<TInnerErrors...>::message();
    }
    inline const abc::string message_with_inner() const
    {
        return m_optError.has_value() ? m_optError->message_with_inner()
                                      : inner_error<TInnerErrors...>::message_with_inner();
    }

    bool operator==(const TError &other) const
    {
        return m_optError.has_value() && other.m_optError.has_value() && m_optError.get() &&
               other.m_optError.get();
    }
    bool operator==(const typename TError::code_t &errorCode) const
    {
        return m_optError.has_value() && m_optError.get() == errorCode;
    }

    template <typename TInnerErrorCode>
    bool operator==(const TInnerErrorCode &errorCode) const
    {
        return m_optError.has_value() && m_optError.get() == errorCode;
    }
};

template <typename TError>
class inner_error<TError>
{
    abc::optional<TError> m_optError;

public:
    using error_t = typename std::remove_const<TError>::type;

public:
    inner_error() = default;
    inline inner_error(const inner_error<TError> &other) : m_optError(other.m_optError) {}
    inline inner_error(const error_t& err) : m_optError(err) {}
    inline inner_error(const typename TError::code_t &errorCode) : m_optError(errorCode) {}

    template <typename TInnerOrError = TError>
    inline const abc::optional<TInnerOrError> &get() const
    {
        return m_optError;
    }
    inline const abc::string message() const
    {
        return m_optError.has_value() ? m_optError->message() : abc::string();
    }
    inline const abc::string message_with_inner() const
    {
        return m_optError.has_value() ? m_optError->message_with_inner() : abc::string();
    }

    bool operator==(const TError &other) const
    {
        return m_optError.has_value() && other.m_optError.has_value() && m_optError.get() &&
               other.m_optError.get();
    }
    bool operator==(const typename TError::code_t &errorCode) const
    {
        return m_optError.has_value() && m_optError.get() == errorCode;
    }
};

template <>
class inner_error<>
{
public:
    inner_error()                                  = default;
    inline inner_error(const inner_error<> &other) = default;

    // inline const abc::optional<void>& get() const { return m_optError; }
    inline const abc::string message() const { return abc::string(); }
    inline const abc::string message_with_inner() const { return abc::string(); }
};

//////////////////////////////////////////////////////////////////////////
}  // namespace detail

/**
Usage:
auto myErr = error<errorCodeEnum, error<innerErrorCodeEnum>>(errCodeEnum(0),
error<innerRrrorCodeEnum>(innerErrorCodeEnum(0), "inner error message"), "error message");
assert(myErr.has_inner<error<innerErrorCodeEnum>>());
assert(myErr.inner<error<innerRrrorCodeEnum>>().code() == innerErrorCodeEnum(0));
assert(myErr.code() == errorCodeEnum(0));
*/
template <typename TErrorCode, typename... TInnerErrors>
class error
{
    static_assert(std::is_enum<TErrorCode>::value, "Invalid error code type: must be an enum");

public:
    using code_t = TErrorCode;

public:
    explicit inline error(const code_t i_code) : m_code(i_code) {}
    inline error(const code_t &i_code, const char *i_message) : m_code(i_code), m_message(i_message)
    {
    }
    inline error(const code_t &i_code, const abc::string &i_message)
        : m_code(i_code), m_message(i_message)
    {
    }
    inline error(const error<code_t, TInnerErrors...> &other)
        : m_code(other.m_code), m_message(other.m_message), m_innerErrors(other.m_innerErrors)
    {
    }

    template <typename TInnerError>
    inline error(const code_t &code, const TInnerError &innerError)
        : m_code(code), m_innerErrors(innerError)
    {
    }
    template <typename TInnerError>
    inline error(const code_t &code, const TInnerError &innerError, const abc::string &message)
        : m_code(code), m_message(message), m_innerErrors(innerError)
    {
    }

    inline code_t            code() const { return m_code; }
    inline const abc::string message() const { return m_message; }
    inline const abc::string message_with_inner() const
    {
        abc::string what = m_innerErrors.message();
        if (!m_message.empty())
        {
            if (!what.empty())
            {
                what += "\n";
            }
            what += m_message;
        }
        return what;
    }

    template <typename TInnerError>
    inline bool has_inner() const
    {
        return m_innerErrors.template get<TInnerError>().has_value();
    }
    template <typename TInnerError>
    inline TInnerError inner() const
    {
        abc::optional<TInnerError> optErr = m_innerErrors.template get<TInnerError>();
        return optErr.value();
        // return m_innerErrors.get<TInnerError>().get();
    }

    bool operator==(const TErrorCode &errorCode) const { return m_code == errorCode; }
    bool operator!=(const TErrorCode &errorCode) const { return !operator==(errorCode); }

protected:
    code_t                               m_code;
    abc::string                          m_message;
    detail::inner_error<TInnerErrors...> m_innerErrors;
};

template <typename TErrorCode>
class error<TErrorCode, void>
{
public:
    using code_t = TErrorCode;

public:
    error(TErrorCode i_code) : m_code(i_code) {}
    error(TErrorCode i_code, const char *i_message) : m_code(i_code), m_message(i_message) {}
    error(TErrorCode i_code, const abc::string &i_message) : m_code(i_code), m_message(i_message) {}

    inline TErrorCode         code() const { return m_code; }
    inline const abc::string &message() const { return m_message; }

protected:
    code_t      m_code;
    abc::string m_message;
};

enum class generic_error_type
{
    generic_error
};

template <typename TPayload, typename TError = error<generic_error_type>>
class result : abc::noncopyable
{
public:
    using payload_t    = TPayload;
    using error_t      = TError;
    using error_code_t = typename error_t::code_t;
    using this_t       = result<payload_t, error_t>;

public:
    inline ~result() { require_checked(); }

    inline result(const result &other)
        : m_optPayload(std::move(other.m_optPayload)), m_optError(std::move(other.m_optError))
    {
    }
    inline result(result &&other)
        : m_optPayload(std::move(other.m_optPayload)), m_optError(std::move(other.m_optError))
    {
    }
    inline this_t &operator=(const this_t &other)
    {
        m_optPayload = std::move(other.m_optPayload);
        m_optError   = std::move(other.m_optError);
    }
    inline this_t &operator=(this_t &&other)
    {
        m_optPayload = std::move(other.m_optPayload);
        m_optError   = std::move(other.m_optError);
    }

    inline result(const payload_t& res) : m_optPayload(res) {}
    inline result(payload_t &&res) : m_optPayload(std::move(res)) {}

	inline result(const error_t& err) : m_optError(err) {}
	inline result(error_t&& err) : m_optError(std::move(err)) {}
    inline result(const error_code_t &error_code) : m_optError(error_t(error_code)) {}
    inline result(const error_code_t &error_code, const abc::string &what)
        : m_optError(error_t(error_code, what))
    {
    }

    inline const error_t &get_error() const
    {
        require_checked();
        return m_optError.value();
    }
    inline const payload_t &get_payload() const
    {
        require_checked();
        return *m_optPayload;
    }
    inline payload_t &&extract_payload()
    {
        require_checked();
        return std::move(m_optPayload->value());
    }

    inline void ignore_result() const { set_checked(); }

    inline bool operator==(success_t i_success) const
    {
        set_checked();
        return !m_optError.has_value();
    }
    inline bool operator!=(success_t i_success) const { return !operator==(i_success); }

    inline bool operator==(const error_code_t &i_errorCode) const
    {
        set_checked();
        return m_optError.has_value() && m_optError->code() == i_errorCode;
    }
    inline bool operator!=(const error_code_t &i_errorCode) const
    {
        return !operator==(i_errorCode);
    }

protected:
	abc::optional<payload_t> m_optPayload;
    abc::optional<error_t>   m_optError;

private:
#ifdef ABC_DEBUG
    mutable bool m_checked = false;
#endif

    void set_checked() const
    {
#ifdef ABC_DEBUG
        m_checked = true;
#endif
    }
    void require_checked() const
    {
#ifdef ABC_DEBUG
        ABC_ASSERT(m_checked, "abc::result has not been checked");
#endif
    }
};

template <typename TError>
class result<void, TError> : abc::noncopyable
{
public:
    using error_t      = TError;
    using error_code_t = typename error_t::code_t;
    using this_t       = result<void, error_t>;

public:
    ~result() { require_checked(); }

    result(success_t) {}
    result(result &&other) : m_optError(std::move(other.m_optError)) {}

    this_t &operator=(const this_t &other) { m_optError = std::move(other.m_optError); }
    this_t &operator=(this_t &&other) { m_optError = std::move(other.m_optError); }

    result(const error_t &err) : m_optError(err) {}
    result(const error_code_t &error_code) : m_optError(error_t(error_code)) {}
    result(const error_code_t &error_code, const abc::string &what)
        : m_optError(error_t(error_code, what))
    {
    }

    const error_t &get_error() const
    {
        require_checked();
        return m_optError.value();
    }

    void ignore_result() const { set_checked(); }

    bool operator==(success_t i_success) const
    {
        set_checked();
        return !m_optError.has_value();
    }
    bool operator!=(success_t i_success) const { return !operator==(i_success); }

    bool operator==(const error_code_t &i_errorCode) const
    {
        set_checked();
        return m_optError.has_value() && m_optError->code() == i_errorCode;
    }
    bool operator!=(const error_code_t &i_errorCode) const { return !operator==(i_errorCode); }

protected:
    abc::optional<error_t> m_optError;

private:
#ifdef ABC_DEBUG
    mutable bool m_checked = false;
#endif

    void set_checked() const
    {
#ifdef ABC_DEBUG
        m_checked = true;
#endif
    }
    void require_checked() const
    {
#ifdef ABC_DEBUG
        ABC_ASSERT(m_checked, "abc::result has not been checked");
#endif
    }
};

//////////////////////////////////////////////////////////////////////////
}  // namespace abc
