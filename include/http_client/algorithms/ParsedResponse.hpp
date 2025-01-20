/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_RESPONSE_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_RESPONSE_HPP

#include "Response.hpp"
#include "ChunkyBodyParser.hpp"
#include <version>
#include <variant>
#include <thread>
#include <string_view>
#include <string>
#include <stdexcept>
#include <span>
#include <ranges>
#include <memory>
#include <iostream>
#include <future>
#include <functional>
#include <fstream>
#include <format>
#include <concepts>
#include <chrono>
#include <charconv>
#include <array>
#include <algorithm>

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

    ParsedResponse(StatusLine p_status_line, std::string p_headers_string = {}, std::vector<Header> p_headers = {}, utils::DataVector p_body_data = {})
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
