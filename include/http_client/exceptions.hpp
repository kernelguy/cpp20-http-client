#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_EXCEPTIONS_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_EXCEPTIONS_HPP

#include <exception>
#include <string>

namespace http_client::errors {

class HttpClientException : public std::exception
{
public:
    explicit HttpClientException(std::string reason)
            : reason_(std::move(reason))
    {
    }

    [[nodiscard]] char const *what() const noexcept override
    {
        return reason_.c_str();
    }

private:
    std::string reason_;
};

/*
	The connection to the server failed in some way.
	For example, there is no internet connection or the server name is invalid.
*/
class ConnectionFailed : public HttpClientException
{
public:
    [[nodiscard]]
    bool get_is_tls_failure() const noexcept
    {
        return is_tls_failure_;
    }

    explicit ConnectionFailed(std::string reason, bool const is_tls_failure = false) noexcept
            : HttpClientException(std::move(reason)),
              is_tls_failure_{is_tls_failure}
    {
    }

private:
    bool is_tls_failure_;
};

class ResponseParsingFailed : public HttpClientException
{
public:
    using HttpClientException::HttpClientException;
};

} // namespace http_client::errors

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_EXCEPTIONS_HPP
