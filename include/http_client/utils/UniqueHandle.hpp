/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_UNIQUEHANDLE_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_UNIQUEHANDLE_HPP

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
	Similar to std::unique_ptr except that non-pointer types can be held
	and that a custom deleter must be specified.

	This is useful for OS handles that are integer types, for example a native socket handle.
	Use C++20 lambdas in unevaluated contexts to specify a deleter, or use an already defined
	functor type.

	Example:
	using DllHandle = utils::UniqueHandle<HMODULE, decltype([](auto& h){ FreeLibrary(h); })>;
*/
template<IsTrivial T, std::invocable<T&> Deleter_, T invalid_handle = T{}>
class UniqueHandle
{
public:
    [[nodiscard]]
    constexpr explicit operator T() const noexcept
    {
        return handle_;
    }

    [[nodiscard]]
    constexpr T get() const noexcept
    {
        return handle_;
    }

    [[nodiscard]]
    constexpr T& get() noexcept
    {
        return handle_;
    }

    [[nodiscard]]
    constexpr T const *operator->() const noexcept
    {
        return &handle_;
    }

    [[nodiscard]]
    constexpr T *operator->() noexcept
    {
        return &handle_;
    }

    [[nodiscard]]
    constexpr T const *operator&() const noexcept
    {
        return &handle_;
    }

    [[nodiscard]]
    constexpr T *operator&() noexcept
    {
        return &handle_;
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return handle_ != invalid_handle;
    }

    [[nodiscard]]
    constexpr bool operator!() const noexcept
    {
        return handle_ == invalid_handle;
    }

    [[nodiscard]]
    constexpr bool operator==(UniqueHandle const&) const noexcept requires std::equality_comparable<T> = default;

    constexpr explicit UniqueHandle(T const handle) noexcept
            : handle_{handle}
    {
    }

    constexpr UniqueHandle& operator=(T const handle)
    {
        close_();
        handle_ = handle;
        return *this;
    }

    constexpr UniqueHandle() = default;

    constexpr ~UniqueHandle()
    {
        close_();
    }

    constexpr UniqueHandle(UniqueHandle&& handle) noexcept
            : handle_{handle.handle_}
    {
        handle.handle_ = invalid_handle;
    }

    constexpr UniqueHandle& operator=(UniqueHandle&& handle) noexcept
    {
        handle_ = handle.handle_;
        handle.handle_ = invalid_handle;
        return *this;
    }

    constexpr UniqueHandle(UniqueHandle const&) = delete;
    constexpr UniqueHandle& operator=(UniqueHandle const&) = delete;

private:
    T handle_{invalid_handle};

    constexpr void close_()
    {
        if (handle_ != invalid_handle) {
            Deleter_{}(handle_);
            handle_ = invalid_handle;
        }
    }
};

} // namespace http_client::utils

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_UNIQUEHANDLE_HPP
