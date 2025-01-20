/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_HEADERS_INTERFACE_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_HEADERS_INTERFACE_HPP

#include "cpp20_http_client.hpp"

namespace http_client::algorithms {

struct ParsedHeadersInterface
{
    virtual ~ParsedHeadersInterface() = default;

    [[nodiscard]] constexpr virtual http_client::algorithms::ParsedResponse const& get_parsed_response() const noexcept = 0;

    /*
        Returns the status code from the response header.
    */
    [[nodiscard]]
    StatusCode get_status_code() const
    {
        return get_parsed_response().status_line.status_code;
    }

    /*
        Returns the status code description from the response header.
    */
    [[nodiscard]]
    std::string_view get_status_message() const
    {
        return get_parsed_response().status_line.status_message;
    }

    /*
        Returns the HTTP version from the response header.
    */
    [[nodiscard]]
    std::string_view get_http_version() const
    {
        return get_parsed_response().status_line.http_version;
    }

    /*
        Returns a const reference to the parsed status line object.
    */
    [[nodiscard]]
    StatusLine const& get_status_line() const
    {
        return get_parsed_response().status_line;
    }

    /*
        Returns the headers of the response as a string.
        The returned string_view shall not outlive this Response object.
    */
    [[nodiscard]]
    std::string_view get_headers_string() const
    {
        return get_parsed_response().headers_string;
    }

    /*
        Returns the headers of the response as Header objects.
        The returned span shall not outlive this Response object.
    */
    [[nodiscard]]
    std::span<Header const> get_headers() const
    {
        return get_parsed_response().headers;
    }

    /*
        Returns a header of the response by its name.
        The returned header shall not outlive this Response object.
    */
    [[nodiscard]]
    std::optional<Header> get_header(std::string_view const name) const
    {
        if (auto const header = http_client::algorithms::find_header_by_name(get_parsed_response().headers, name)) {
            return *header;
        }
        else
            return {};
    }

    /*
        Returns a header value of the response by its name.
        The returned std::string_view shall not outlive this Response object.
    */
    [[nodiscard]]
    std::optional<std::string_view> get_header_value(std::string_view const name) const
    {
        if (auto const header = http_client::algorithms::find_header_by_name(get_parsed_response().headers, name)) {
            return header->value;
        }
        else
            return {};
    }
};

} // namespace http_client::algorithms

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_HEADERS_INTERFACE_HPP
