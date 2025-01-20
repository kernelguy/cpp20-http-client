/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_URL_COMPONENTS_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_URL_COMPONENTS_HPP

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
	The result of the split_url function.
*/
struct UrlComponents {
	Protocol protocol{Protocol::Unknown};
	std::string_view host;
	Port port{default_port_for_protocol(Protocol::Unknown)};
	std::string_view path;
};
struct HostAndPort {
	std::string_view host;
	std::optional<Port> port;
};

/*
	Splits a domain name into a name and an optional port.
	For example, "localhost:8080" returns "localhost" and 8080, while
	"google.com" returns "google.com" and no port (std::nullopt).
*/
[[nodiscard]]
inline HostAndPort split_domain_name(std::string_view const domain_name)
{
	if (auto const colon_position = domain_name.rfind(':');
		colon_position != std::string_view::npos)
	{
		if (auto const port = string_to_integral<Port>(domain_name.substr(colon_position + 1)))
		{
			return HostAndPort{
				.host{domain_name.substr(0, colon_position)},
				.port{port}
			};
		}
		else
		{
			return HostAndPort{
				.host{domain_name.substr(0, colon_position)},
				.port = std::nullopt
			};
		}
	}
	return HostAndPort{
		.host{domain_name},
		.port = std::nullopt
	};
}

/*
	Splits an URL into its components.
*/
[[nodiscard]]
inline UrlComponents split_url(std::string_view const url) noexcept {
	using namespace std::string_view_literals;

	if (url.empty()) {
		return {};
	}

	auto result = UrlComponents{};

	constexpr auto whitespace_characters = " \t\r\n"operator;

	// Find the start position of the protocol.
	auto start_position = url.find_first_not_of(whitespace_characters);
	if (start_position == std::string_view::npos) {
		return {};
	}

	constexpr auto protocol_suffix = "://"operator;

	// Find the end position of the protocol.
	if (auto const position = url.find(protocol_suffix, start_position);
		position != std::string_view::npos)
	{
		result.protocol = get_protocol_from_string(url.substr(start_position, position - start_position));
		result.port = default_port_for_protocol(result.protocol);

		// The start position of the domain name.
		start_position = position + protocol_suffix.length();
	}

	// Find the end position of the domain name and start of the path.
	if (auto const slash_position = url.find('/', start_position);
		slash_position != std::string_view::npos)
	{
		auto [host, port] = split_domain_name(url.substr(start_position, slash_position - start_position));

		result.host = host;

		if (port) {
			result.port = *port;
		}

		start_position = slash_position;
	}
	else {
		// There was nothing after the domain name.
		auto [host, port] = split_domain_name(url.substr(start_position));

		result.host = host;

		if (port) {
			result.port = *port;
		}

		result.path = "/"operator;
		return result;
	}

	// Find the end position of the path.
	auto const end_position = url.find_last_not_of(whitespace_characters) + 1;
	result.path = url.substr(start_position, end_position - start_position);
	return result;
}

/*
	Returns the file name part of a URL (or file path with only forward slashes).
*/
[[nodiscard]]
constexpr std::string_view extract_filename(std::string_view const url)
{
	if (auto const slash_pos = url.rfind('/');
		slash_pos != std::string_view::npos)
	{
		if (auto const question_mark_pos = url.find('?', slash_pos + 1);
			question_mark_pos != std::string_view::npos)
		{
			return url.substr(slash_pos + 1, question_mark_pos - slash_pos - 1);
		}

		return url.substr(slash_pos + 1);
	}
	return {};
}

/*
	Returns whether character is allowed in a URI-encoded string or not.
*/
[[nodiscard]]
constexpr bool get_is_allowed_uri_character(char const character) noexcept {
	constexpr auto other_characters = std::string_view{"%-._~:/?#[]@!$&'()*+,;="};

	return (character >= '0' && character <= '9') ||
		(character >= 'a' && character <= 'z') ||
		(character >= 'A' && character <= 'Z') ||
		other_characters.find(character) != std::string_view::npos;
}

/*
	Returns the URI-encoded equivalent of uri.
*/
[[nodiscard]]
inline std::string uri_encode(std::string_view const uri) {
	auto result_string = std::string();
	result_string.reserve(uri.size());

	for (auto const character : uri) {
		if (get_is_allowed_uri_character(character)) {
			result_string += character;
		}
		else {
			result_string += "%xx";
			std::to_chars(
				&result_string.back() - 1,
				&result_string.back() + 1,
				static_cast<unsigned char>(character),
				16
			);
		}
	}
	return result_string;
}

} // namespace http_client::utils

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_URL_COMPONENTS_HPP
