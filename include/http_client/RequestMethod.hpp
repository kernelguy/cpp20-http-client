#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_REQUEST_METHOD_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_REQUEST_METHOD_HPP

#include <string_view>
#include "utils/template-helpers.hpp"

namespace http_client {

/*
	Enumeration of the different HTTP request methods that can be used.
*/
enum class RequestMethod
{
    Connect,
    Delete,
    Get,
    Head,
    Options,
    Patch,
    Post,
    Put,
    Trace,
};

/*
	Converts a RequestMethod to its uppercase string equivalent.
	For example, RequestMethod::Get becomes std::string_view{"GET"}.
*/
[[nodiscard]]
inline std::string_view request_method_to_string(RequestMethod const method)
{
    using
    enum RequestMethod;
    switch (method) {
        case Connect:
            return "CONNECT";
        case Delete:
            return "DELETE";
        case Get:
            return "GET";
        case Head:
            return "HEAD";
        case Options:
            return "OPTIONS";
        case Patch:
            return "PATCH";
        case Post:
            return "POST";
        case Put:
            return "PUT";
        case Trace:
            return "TRACE";
    }
    utils::unreachable();
}

} // namespace http_client

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_REQUEST_METHOD_HPP
