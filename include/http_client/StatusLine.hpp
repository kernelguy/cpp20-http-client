
#ifndef CPP20HTTPCLIENT_INCLUDE_HTTP_CLIENT_STATUS_LINE_HPP
#define CPP20HTTPCLIENT_INCLUDE_HTTP_CLIENT_STATUS_LINE_HPP

#include "StatusCode.hpp"
#include "utils/template-helpers.hpp"
#include <string>

namespace http_client {

struct StatusLine
{
    std::string http_version;
    StatusCode status_code = StatusCode::Unknown;
    std::string status_message;

    [[nodiscard]]
    bool operator==(StatusLine const&) const noexcept = default;
};


} // namespace http_client

#endif //CPP20HTTPCLIENT_INCLUDE_HTTP_CLIENT_STATUS_LINE_HPP
