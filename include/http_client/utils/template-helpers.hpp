/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_TEMPLATE_HELPERS_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_TEMPLATE_HELPERS_HPP

#	include <openssl/ssl.h>
#	include <openssl/err.h>
#	include <unistd.h>
#	include <sys/socket.h>
#	include <netinet/tcp.h>
#	include <netdb.h>
#	include <fcntl.h>
#	include <errno.h>
#	include <arpa/inet.h>
#include <system_error>
#include <cstring>
#include <cassert>
#include "exceptions.hpp"
#include "Socket.hpp"
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
	This is everything that doesn't have anything to do with the core functionality,
	but are utilities that are used within the library.
*/

void enable_utf8_console()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    // Pretty much everyone else uses utf-8 by default.
}

[[noreturn]]
inline void unreachable()
{
    std::cerr << "Reached an unreachable code path, exiting.\n";
    exit(1);
}

/*
	Prints an error message to the error output stream and exits the program.
*/
[[noreturn]]
inline void panic(std::string_view const message)
{
    std::cerr << message << '\n';
    exit(1);
}

concept IsAnyOf = (std::same_as<T, U> || ...);
concept IsTrivial = std::is_trivial_v<T>;
concept IsByte = IsAnyOf<std::remove_cvref_t<T>, std::byte, char, unsigned char>;
concept IsInputRangeOf = std::ranges::input_range<Range_> && std::same_as<std::ranges::range_value_t<Range_>, Value_>;
concept IsSizedRangeOf = IsInputRangeOf<Range_, Value_> && std::ranges::sized_range<Range_>;


/*
	This can be called when the program reaches a path that should never be reachable.
	It prints error output and exits the program.
*/
#ifdef __cpp_lib_source_location
[[noreturn]]
inline void unreachable(std::source_location const& source_location = std::source_location::current()) {
	std::cerr << std::format("Reached an unreachable code path in file {}, in function {}, on line {}.\n",
		source_location.file_name(), source_location.function_name(), source_location.line());
	std::exit(1);
}
#else
#endif

/*
	Converts a range of contiguous characters to a std::basic_string_view.

	TODO: Remove this in C++23; std::views::split will return contiguous ranges and std::basic_string_view will have a range constructor.
*/
constexpr auto range_to_string_view = []<
        /*
            std::views::split returns a range of ranges.
            The ranges unfortunately are not std::ranges::contiguous_range
            even when the base type is contiguous, so we can't use that constraint.

            This will be fixed :^D
            http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2210r2.html
        */
        IsInputRangeOf<char> Range_
> (Range_&& range) {
    return std::string_view{
            &*std::ranges::begin(range),
            static_cast<std::string_view::size_type>(std::ranges::distance(range))
    };
};

//---------------------------------------------------------

void enable_utf8_console();

//---------------------------------------------------------

/*
	Copies a sized range to a std::basic_string of any type.
*/
template<IsSizedRangeOf<char> Range_>
[[nodiscard]]
inline std::string range_to_string(Range_ const& range) {
    auto result = std::string(range.size(), char{});
    std::ranges::copy(range, std::ranges::begin(result));
    return result;
}

/*
	Copies a range of unknown size to a std::basic_string of any type.
*/
template<IsInputRangeOf<char> Range_>
[[nodiscard]]
inline std::string range_to_string(Range_ const& range) {
    auto result = std::string();
    std::ranges::copy(range, std::back_inserter(result));
    return result;
}

/*
	Reinterprets a span of any byte-sized trivial type as a string view of a specified byte-sized character type.
*/
template<IsByte Byte_>
[[nodiscard]]
std::string_view data_to_string(std::span<Byte_> const data) {
    return std::string_view{reinterpret_cast<char const*>(data.data()), data.size()};
}
/*
	Reinterprets a string view of any byte-sized character type as a span of any byte-sized trivial type.
*/
template<IsByte Byte_>
[[nodiscard]]
std::span<Byte_ const> string_to_data(std::string_view const string) {
    return std::span{reinterpret_cast<Byte_ const*>(string.data()), string.size()};
}

//---------------------------------------------------------

using DataVector = std::vector<std::byte>;

//---------------------------------------------------------

template<std::movable T>
void append_to_vector(std::vector<T>& vector, std::span<T const> const data) {
    vector.insert(vector.end(), data.begin(), data.end());
}

//---------------------------------------------------------

template<typename T>
concept IsByteData = IsByte<T> || (std::ranges::range<T> && IsByte<std::ranges::range_value_t<T>>);

/*
	Returns the size of any trivial byte-sized element or range of trivial byte-sized elements.
*/
template<IsByteData T>
[[nodiscard]]
std::size_t size_of_byte_data(T const& data) {
    if constexpr (std::ranges::range<T>) {
        return static_cast<std::size_t>(std::ranges::distance(data));
    }
    else {
        return sizeof(data);
    }
}

/*
	Copies any type of trivial byte-sized element(s) from data to range.
*/
template<IsByteData Data_, std::ranges::contiguous_range Range_, IsByte RangeValue_ = std::ranges::range_value_t<Range_>>
[[nodiscard]]
auto copy_byte_data(Data_ const& data, Range_& range)
-> std::ranges::iterator_t<Range_>
{
    if constexpr (IsByte<Data_>) {
        *std::ranges::begin(range) = *reinterpret_cast<RangeValue_ const*>(&data);
        return std::ranges::begin(range) + 1;
    }
    else {
        return std::ranges::copy(std::span{
                reinterpret_cast<RangeValue_ const*>(std::ranges::data(data)),
                std::ranges::size(data)
        }, std::ranges::begin(range)).out;
    }
}

/*
	Concatenates any kind of sequence of trivial byte-sized elements like char and std::byte.
	The arguments can be individual bytes and/or ranges of bytes.
	Returns a utils::DataVector (std::vector<std::byte>).
*/
template<IsByteData ... T>
[[nodiscard]]
DataVector concatenate_byte_data(T const& ... arguments) {
    auto buffer = DataVector((size_of_byte_data(arguments) + ...));
    auto buffer_span = std::span{buffer};
    ((buffer_span = std::span{copy_byte_data(arguments, buffer_span), buffer_span.end()}), ...);
    return buffer;
}

//---------------------------------------------------------

/*
	Parses a string as an integer type in a given base.
	For more details, see std::from_chars. This is just an abstraction layer on top of it.
*/
template<std::integral T>
[[nodiscard]]
std::optional<T> string_to_integral(std::string_view const string, int const base = 10)
{
    auto number_result = T{};
    if (std::from_chars(string.data(), string.data() + string.size(), number_result, base).ec == std::errc{}) {
        return number_result;
    }
    return {};
}

//---------------------------------------------------------

template<std::ranges::contiguous_range DataRange_> requires IsByte<std::ranges::range_value_t<DataRange_>>
void write_to_file(DataRange_ const& data, std::string const& file_name) {
    // std::string because std::ofstream does not take std::string_view.
    auto file_stream = std::ofstream{file_name, std::ios::binary};
    file_stream.write(reinterpret_cast<char const*>(std::ranges::data(data)), static_cast<std::streamsize>(std::ranges::size(data)));
}

//---------------------------------------------------------

constexpr auto filter_true = std::views::filter([](auto const& x){ return static_cast<bool>(x); });
constexpr auto dereference_move = std::views::transform([](auto&& x) { return std::move(*x); });

/*
	Transforms a range of chars into its lowercase equivalent.
*/
constexpr auto ascii_lowercase_transform = std::views::transform([](char const c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
});

/*
	Returns whether lhs and rhs are equal, regardless of casing, assuming both are encoded in ASCII.
*/
[[nodiscard]]
constexpr bool equal_ascii_case_insensitive(std::string_view const lhs, std::string_view const rhs) noexcept {
    return std::ranges::equal(lhs | ascii_lowercase_transform, rhs | ascii_lowercase_transform);
}

} // namespace http_client::utils

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_TEMPLATE_HELPERS_HPP