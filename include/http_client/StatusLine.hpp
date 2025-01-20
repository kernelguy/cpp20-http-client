/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_INCLUDE_HTTP_CLIENT_STATUS_LINE_HPP
#define CPP20HTTPCLIENT_INCLUDE_HTTP_CLIENT_STATUS_LINE_HPP

namespace http_client {

struct StatusLine
{
    std::string http_version;
    StatusCode status_code = StatusCode::Unknown;
    std::string status_message;

    [[nodiscard]]
    bool operator==(StatusLine const&) const noexcept = default;
};

[[nodiscard]]
inline StatusLine parse_status_line(std::string_view const line) {
    auto status_line = StatusLine{};

    auto cursor = std::size_t{};

    if (auto const http_version_end = line.find(' '); http_version_end != std::string_view::npos)
    {
        status_line.http_version = line.substr(0, http_version_end);
        cursor = http_version_end + 1;
    }
    else return status_line;

    if (auto const status_code_end = line.find(' ', cursor); status_code_end != std::string_view::npos)
    {
        if (auto const status_code = utils::string_to_integral<int>(line.substr(cursor, status_code_end)))
        {
            status_line.status_code = static_cast<StatusCode>(*status_code);
        }
        else return status_line;
        cursor = status_code_end + 1;
    }
    else return status_line;

    status_line.status_message = line.substr(cursor, line.find_last_not_of("\r\n ") + 1 - cursor);
    return status_line;
}


} // namespace http_client

#endif //CPP20HTTPCLIENT_INCLUDE_HTTP_CLIENT_STATUS_LINE_HPP
