#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_RESPONSE_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_RESPONSE_HPP

#include "../StatusLine.hpp"
#include "../Header.hpp"

namespace http_client::algorithms {

struct ParsedResponse
{
    StatusLine status_line;
    std::string headers_string;
    std::vector<Header> headers; // Points into headers_string
    utils::DataVector body_data;

    [[nodiscard]]
    bool operator==(ParsedResponse const&) const noexcept = default;

    ParsedResponse() = default;

    explicit ParsedResponse(StatusLine p_status_line, std::string p_headers_string = {}, std::vector<Header> p_headers = {}, utils::DataVector p_body_data = {})
            : status_line{std::move(p_status_line)}, headers_string(std::move(p_headers_string)), headers(std::move(p_headers)), body_data(std::move(p_body_data))
    {
    }

    ParsedResponse(ParsedResponse&&) = default;
    ParsedResponse& operator=(ParsedResponse&&) = default;

    ParsedResponse(ParsedResponse const&) = delete;
    ParsedResponse& operator=(ParsedResponse const&) = delete;
};

} // namespace http_client::algorithms

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_RESPONSE_HPP
