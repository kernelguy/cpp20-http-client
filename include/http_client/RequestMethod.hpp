/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_REQUEST_METHOD_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_REQUEST_METHOD_HPP

#include "Request.hpp"
#include "ResponseProgress.hpp"
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

/*
	Enumeration of the different HTTP request methods that can be used.
*/
enum class RequestMethod {
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
inline std::string_view request_method_to_string(RequestMethod const method) {
    using enum RequestMethod;
    switch (method) {
        case Connect: return "CONNECT";
        case Delete:  return "DELETE";
        case Get:     return "GET";
        case Head:    return "HEAD";
        case Options: return "OPTIONS";
        case Patch:   return "PATCH";
        case Post:    return "POST";
        case Put:     return "PUT";
        case Trace:   return "TRACE";
    }
    utils::unreachable();
}
#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_REQUEST_METHOD_HPP
