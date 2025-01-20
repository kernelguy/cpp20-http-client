/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSECALLBACKS_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSECALLBACKS_HPP

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

struct ResponseCallbacks {
	std::function<void(ResponseProgressRaw&)> handle_raw_progress;
	std::function<void(ResponseProgressHeaders&)> handle_headers;
	std::function<void(ResponseProgressBody&)> handle_body_progress;
	std::function<void(Response&)> handle_finish;
	std::function<void()> handle_stop;
};
#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSECALLBACKS_HPP
