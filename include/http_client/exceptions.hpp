/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_EXCEPTIONS_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_EXCEPTIONS_HPP

#include "RequestMethod.hpp"
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
	The connection to the server failed in some way.
	For example, there is no internet connection or the server name is invalid.
*/
class ConnectionFailed : public std::exception {
public:
	[[nodiscard]]
	char const* what() const noexcept override {
		return reason_.c_str();
	}

	[[nodiscard]]
	bool get_is_tls_failure() const noexcept {
		return is_tls_failure_;
	}

	ConnectionFailed(std::string reason, bool const is_tls_failure = false) noexcept :
		reason_(std::move(reason)),
		is_tls_failure_{is_tls_failure}
	{}

private:
	std::string reason_;
	bool is_tls_failure_;
};

class ResponseParsingFailed : public std::exception {
public:
	[[nodiscard]]
	char const* what() const noexcept override {
		return reason_.c_str();
	}

	ResponseParsingFailed(std::string reason) :
		reason_(std::move(reason))
	{}

private:
	std::string reason_;
};

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_EXCEPTIONS_HPP
