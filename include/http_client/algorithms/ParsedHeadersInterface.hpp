#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_HEADERS_INTERFACE_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_HEADERS_INTERFACE_HPP

#include "ParsedResponse.hpp"
#include <http_client/Header.hpp>
#include <http_client/StatusLine.hpp>

namespace http_client::algorithms {

[[nodiscard]]
constexpr std::optional<Header> parse_header(std::string_view const line) {
    /*
        "An HTTP header consists of its case-insensitive name followed by a colon (:),
        then by its value. Whitespace before the value is ignored."
        (https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers)

        So we're just ignoring whitespace before the value, and after because there may be
        an \r there if the line endings are CRLF.
    */

    auto const colon_pos = line.find(':');
    if (colon_pos == std::string_view::npos) {
        return {};
    }

    constexpr auto whitespace_characters = std::string_view{" \t\r"};

    auto const value_start = line.find_first_not_of(whitespace_characters, colon_pos + 1);
    if (value_start == std::string_view::npos) {
        return {};
    }

    // This will never be npos, assuming the header
    // string isn't mutated by some other thread.
    auto const value_end = line.find_last_not_of(whitespace_characters);

    return Header{
            .name = line.substr(0, colon_pos),
            .value = line.substr(value_start, value_end + 1 - value_start)
    };
}

[[nodiscard]]
inline StatusLine parse_status_line(std::string_view const line) {
    auto status_line = StatusLine{};

    auto cursor = std::size_t{};

    if (auto const http_version_end = line.find(' '); http_version_end != std::string_view::npos)
    {
        status_line.http_version = line.substr(0, http_version_end);
        cursor = http_version_end + 1;
    }
    else return status_line;

    if (auto const status_code_end = line.find(' ', cursor); status_code_end != std::string_view::npos)
    {
        if (auto const status_code = utils::string_to_integral<int>(line.substr(cursor, status_code_end)))
        {
            status_line.status_code = static_cast<StatusCode>(*status_code);
        }
        else return status_line;
        cursor = status_code_end + 1;
    }
    else return status_line;

    status_line.status_message = line.substr(cursor, line.find_last_not_of("\r\n ") + 1 - cursor);
    return status_line;
}

[[nodiscard]]
inline std::vector<Header> parse_headers_string(std::string_view const headers)
{
    auto result = std::vector<Header>();

    std::ranges::copy(
            headers
            | std::views::split('\n') | std::views::transform(utils::range_to_string_view)
            | std::views::transform(parse_header) | utils::filter_true | utils::dereference_move,
            std::back_inserter(result)
    );

    return result;
}

template<std::ranges::input_range Range_, IsHeader Header_ = std::ranges::range_value_t<Range_>>
[[nodiscard]]
inline Header_ const* find_header_by_name(Range_ const& headers, std::string_view const name)
{
    auto const lowercase_name_to_search = utils::range_to_string(
            name | utils::ascii_lowercase_transform
    );
    auto const pos = std::ranges::find_if(headers, [&](Header_ const& header) {
        return std::ranges::equal(lowercase_name_to_search, header.name | utils::ascii_lowercase_transform);
    });
    if (pos == std::ranges::end(headers)) {
        return nullptr;
    }
    else {
        return &*pos;
    }
}

struct ParsedHeadersInterface
{
    virtual ~ParsedHeadersInterface() = default;

    [[nodiscard]] constexpr virtual http_client::algorithms::ParsedResponse const& get_parsed_response() const noexcept = 0;

    /*
        Returns the status code from the response header.
    */
    [[nodiscard]]
    StatusCode get_status_code() const
    {
        return get_parsed_response().status_line.status_code;
    }

    /*
        Returns the status code description from the response header.
    */
    [[nodiscard]]
    std::string_view get_status_message() const
    {
        return get_parsed_response().status_line.status_message;
    }

    /*
        Returns the HTTP version from the response header.
    */
    [[nodiscard]]
    std::string_view get_http_version() const
    {
        return get_parsed_response().status_line.http_version;
    }

    /*
        Returns a const reference to the parsed status line object.
    */
    [[nodiscard]]
    StatusLine const& get_status_line() const
    {
        return get_parsed_response().status_line;
    }

    /*
        Returns the headers of the response as a string.
        The returned string_view shall not outlive this Response object.
    */
    [[nodiscard]]
    std::string_view get_headers_string() const
    {
        return get_parsed_response().headers_string;
    }

    /*
        Returns the headers of the response as Header objects.
        The returned span shall not outlive this Response object.
    */
    [[nodiscard]]
    std::span<Header const> get_headers() const
    {
        return get_parsed_response().headers;
    }

    /*
        Returns a header of the response by its name.
        The returned header shall not outlive this Response object.
    */
    [[nodiscard]]
    std::optional<Header> get_header(std::string_view const name) const
    {
        if (auto const header = http_client::algorithms::find_header_by_name(get_parsed_response().headers, name)) {
            return *header;
        }
        else
            return {};
    }

    /*
        Returns a header value of the response by its name.
        The returned std::string_view shall not outlive this Response object.
    */
    [[nodiscard]]
    std::optional<std::string_view> get_header_value(std::string_view const name) const
    {
        if (auto const header = http_client::algorithms::find_header_by_name(get_parsed_response().headers, name)) {
            return header->value;
        }
        else
            return {};
    }
};

} // namespace http_client::algorithms

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PARSED_HEADERS_INTERFACE_HPP
