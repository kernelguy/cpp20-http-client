/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PROTOCOL_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PROTOCOL_HPP

#include "utils/template-helpers.hpp"

namespace http_client {

using Port = int;

/*
	An enumeration of the transfer protocols that are supported by the library.
*/
enum class Protocol : Port
{
    Http = 80, Https = 443, Unknown = -1,
};

/*
	Returns the default port corresponding to the specified protocol.
*/
[[nodiscard]]
constexpr Port default_port_for_protocol(Protocol const protocol) noexcept {
    return static_cast<Port>(protocol);
}

[[nodiscard]]
constexpr bool is_protocol_tls_encrypted(Protocol const protocol) noexcept {
    return protocol == Protocol::Https;
}

/*
	Returns the protocol that corresponds to the specified case-insensitive string.
	For example, "http" converts to Protocol::Http.
*/
[[nodiscard]]
constexpr Protocol get_protocol_from_string(std::string_view const protocol_string) noexcept {
    if (utils::equal_ascii_case_insensitive(protocol_string, "http")) {
        return Protocol::Http;
    }
    else if (utils::equal_ascii_case_insensitive(protocol_string, "https")) {
        return Protocol::Https;
    }
    return Protocol::Unknown;
}

} // namespace http_client

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PROTOCOL_HPP
