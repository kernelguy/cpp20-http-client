/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_CLEANUP_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_CLEANUP_HPP

#include "template-helpers.hpp"
#include "Protocol.hpp"
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

namespace http_client::utils {

/*
	Used to invoke a lambda at the end of a scope.
*/
template<std::invocable T>
class [[nodiscard]] Cleanup
{
public:
    [[nodiscard]] Cleanup(T&& callable)
            : callable_{std::forward<T>(callable)}
    {
    }

    Cleanup() = delete;

    ~Cleanup()
    {
        callable_();
    }

    Cleanup(Cleanup&&) noexcept = delete;
    Cleanup& operator=(Cleanup&&) noexcept = delete;

    Cleanup(Cleanup const&) = delete;
    Cleanup& operator=(Cleanup const&) = delete;

private:
    T callable_;
};

} // namespace http_client::utils

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_CLEANUP_HPP
