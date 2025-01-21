/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_HELPERS_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_HELPERS_HPP

#include "../Socket.hpp"
#include "ResponseCallbacks.hpp"
#include "ResponseParser.hpp"

namespace http_client::algorithms {

template<std::size_t buffer_size = (std::size_t{1} << 12)>
[[nodiscard]]
inline Response receive_response(http_client::Socket const &&socket, std::string &&url,
                                 http_client::algorithms::ResponseCallbacks &&callbacks,
                                 const std::chrono::steady_clock::time_point &&start_time_point)
{
    auto has_stopped = false;
    callbacks.handle_stop = [&has_stopped] { has_stopped = true; };

    auto response_parser = http_client::algorithms::ResponseParser{callbacks};

    auto read_buffer = std::array<std::byte, buffer_size>();

    while (!has_stopped) {
        if (auto const read_result = socket.read(read_buffer);
                std::holds_alternative<std::size_t>(read_result)) {
            if (auto parse_result = response_parser.parse_new_data(
                    std::span{read_buffer}.first(std::get<std::size_t>(read_result))
            )) {
                // Calculate the total total duration
                // Total time = End time - Start time
                const auto end_time_point = std::chrono::steady_clock::now();
                const std::chrono::duration<double, std::milli> total_time_duration = end_time_point - start_time_point;
                // Create Response object
                auto response = Response{std::move(*parse_result), std::move(url), std::move(total_time_duration)};
                if (callbacks.handle_finish) {
                    callbacks.handle_finish(response);
                }
                return response;
            }
        } else throw http_client::errors::ConnectionFailed{"The peer closed the connection unexpectedly"};
    }

    http_client::utils::unreachable();
}

} // namespace http_client::algorithms

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_HELPERS_HPP
