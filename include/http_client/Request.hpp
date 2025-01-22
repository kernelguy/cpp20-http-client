#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_REQUEST_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_REQUEST_HPP

#include <format>
#include <functional>
#include <future>
#include <string_view>
#include "algorithms/response-helpers.hpp"
#include "algorithms/ResponseCallbacks.hpp"
#include "algorithms/ResponseProgress.hpp"
#include "Header.hpp"
#include "RequestMethod.hpp"
#include "Socket.hpp"
#include "utils/UrlComponents.hpp"

namespace http_client {

/*
	Represents a HTTP request.
	It is created by calling any of the HTTP verb functions (http_client::get, http_client::post, http_client::put ...)
*/
class Request
{
public:
    /*
        Adds headers to the request as a string.
        These are in the format: "NAME: [ignored whitespace] VALUE"
        The string can be multiple lines for multiple headers.
        Non-ASCII bytes are considered opaque data,
        according to the HTTP specification.
    */
    [[nodiscard]]
    Request &&add_headers(std::string_view const headers_string) &&
    {
        if (headers_string.empty()) {
            return std::move(*this);
        }

        headers_ += headers_string;
        if (headers_string.back() != '\n') {
            headers_ += "\r\n"; // CRLF is the correct line ending for the HTTP protocol
        }

        return std::move(*this);
    }

    /*
        Adds headers to the request.
    */
    template<IsHeader Header_, std::size_t extent = std::dynamic_extent>
    [[nodiscard]]
    Request &&add_headers(std::span<Header_ const, extent> const headers) &&
    {
        auto headers_string = std::string{};
        headers_string.reserve(headers.size() * 128);

        for (auto const &header: headers) {
            headers_string += std::format("{}: {}\r\n", header.name, header.value);
        }

        return std::move(*this).add_headers(headers_string);
    }

    /*
        Adds headers to the request.
    */
    [[nodiscard]]
    Request &&add_headers(std::initializer_list<Header const> const headers) &&
    {
        return std::move(*this).add_headers(std::span{headers});
    }

    /*
        Adds headers to the request.
        This is a variadic template that can take any number of headers.
    */
    template<IsHeader ... Header_>
    [[nodiscard]]
    Request&& add_headers(Header_ &&... p_headers) &&
    {
        auto const headers = std::array{Header{p_headers}...};
        return std::move(*this).add_headers(std::span{headers});
    }

    /*
        Adds a single header to the request.
        Equivalent to add_headers with a single Header argument.
    */
    [[nodiscard]]
    Request&& add_header(Header const &header) &&
    {
        return std::move(*this).add_headers(std::format("{}: {}", header.name, header.value));
    }

    /*
        Sets the content of the request as a sequence of bytes.
    */
    template<utils::IsByte Byte_>
    [[nodiscard]]
    Request &&set_body(std::span<Byte_ const> const body_data) &&
    {
        body_.resize(body_data.size());
        if constexpr (std::same_as<Byte_, std::byte>) {
            std::ranges::copy(body_data, body_.begin());
        } else {
            std::ranges::copy(std::span{reinterpret_cast<std::byte const *>(body_data.data()), body_data.size()},
                              body_.begin());
        }
        return std::move(*this);
    }

    /*
        Sets the content of the request as a string view.
    */
    [[nodiscard]]
    Request &&set_body(std::string_view const body_data) &&
    {
        return std::move(*this).set_body(utils::string_to_data<std::byte>(body_data));
    }

    [[nodiscard]]
    Request &&set_raw_progress_callback(std::function<void(algorithms::ResponseProgressRaw & )> callback) &&
    {
        callbacks_.handle_raw_progress = std::move(callback);
        return std::move(*this);
    }

    [[nodiscard]]
    Request &&set_headers_callback(std::function<void(algorithms::ResponseProgressHeaders & )> callback) &&
    {
        callbacks_.handle_headers = std::move(callback);
        return std::move(*this);
    }

    [[nodiscard]]
    Request &&set_body_progress_callback(std::function<void(algorithms::ResponseProgressBody & )> callback) &&
    {
        callbacks_.handle_body_progress = std::move(callback);
        return std::move(*this);
    }

    [[nodiscard]]
    Request &&set_finish_callback(std::function<void(Response &)> callback) &&
    {
        callbacks_.handle_finish = std::move(callback);
        return std::move(*this);
    }

    // Note: send and send_async are not [[nodiscard]] because callbacks
    // could potentially be used exclusively to handle the response.

    /*
        Sends the request and blocks until the response has been received.
    */
    Response send() &&
    {
        return algorithms::receive_response<>(send_and_get_receive_socket_(), std::move(url_), std::move(callbacks_),
                                              std::move(start_time_point_));
    }

    /*
        Sends the request and blocks until the response has been received.

        The buffer_size template parameter specifies the size of the buffer that data
        from the server is read into at a time. If it is small, then data will be received
        in many times in smaller pieces, with some time cost. If it is big, then
        data will be read few times but in large pieces, with more memory cost.
    */
    template<std::size_t buffer_size>
    Response send() &&
    {
        return algorithms::receive_response<buffer_size>(send_and_get_receive_socket_(), std::move(url_),
                                                         std::move(callbacks_), std::move(start_time_point_));
    }

    /*
        Sends the request and returns immediately after the data has been sent.
        The returned future receives the response asynchronously.
    */
    std::future<Response> send_async() &&
    {
        return std::async(&algorithms::receive_response<>, send_and_get_receive_socket_(), std::move(url_),
                          std::move(callbacks_), std::move(start_time_point_));
    }

    /*
        Sends the request and returns immediately after the data has been sent.
        The returned future receives the response asynchronously.

        The buffer_size template parameter specifies the size of the buffer that data
        from the server is read into at a time. If it is small, then data will be received
        many times in smaller pieces, with some time cost. If it is big, then
        data will be read few times but in large pieces, with more memory cost.
    */
    template<std::size_t buffer_size>
    std::future<Response> send_async() &&
    {
        return std::async(&algorithms::receive_response<buffer_size>, send_and_get_receive_socket_(), std::move(url_),
                          std::move(callbacks_), std::move(start_time_point_));
    }

    Request() = delete;

    ~Request() = default;

    Request(Request &&) noexcept = default;

    Request &operator=(Request &&) noexcept = default;

    Request(Request const &) = delete;

    Request &operator=(Request const &) = delete;

private:
    [[nodiscard]]
    Socket send_and_get_receive_socket_()
    {
        // Start duration time measurement
        start_time_point_ = std::chrono::steady_clock::now();

        auto socket = open_socket(url_components_.host, url_components_.port,
                                  utils::is_protocol_tls_encrypted(url_components_.protocol));

        using namespace std::string_view_literals;

        if (!body_.empty()) {
            headers_ += std::format("Transfer-Encoding: identity\r\nContent-Length: {}\r\n", body_.size());
        }

        auto const request_data = utils::concatenate_byte_data(
                request_method_to_string(method_),
                ' ',
                url_components_.path,
                " HTTP/1.1\r\nHost: "sv,
                url_components_.host,
                headers_,
                "\r\n"sv,
                body_
        );
        socket.write(request_data);

        return socket;
    }

    RequestMethod method_;

    std::string url_;
    utils::UrlComponents url_components_;

    std::string headers_{"\r\n"};
    utils::DataVector body_;

    algorithms::ResponseCallbacks callbacks_;

    std::chrono::steady_clock::time_point start_time_point_;

    Request(RequestMethod const method, std::string_view const url, Protocol const default_protocol) :
            method_{method},
            url_{utils::uri_encode(url)},
            url_components_{utils::split_url(std::string_view{url_})}
    {
        if (url_components_.protocol == Protocol::Unknown) {
            url_components_.protocol = default_protocol;
        }
        if (url_components_.port == utils::default_port_for_protocol(Protocol::Unknown)) {
            url_components_.port = utils::default_port_for_protocol(url_components_.protocol);
        }
    }

    friend Request get(std::string_view, Protocol);

    friend Request post(std::string_view, Protocol);

    friend Request put(std::string_view, Protocol);

    friend Request make_request(RequestMethod, std::string_view, Protocol);
};


/*
	Creates a GET request.
	url is a URL to the server or resource that the request targets.
	If url contains a protocol prefix, it is used. Otherwise, default_protocol is used.
*/
[[nodiscard]]
inline Request get(std::string_view const url, Protocol const default_protocol = Protocol::Http)
{
    return Request{RequestMethod::Get, url, default_protocol};
}

/*
	Creates a POST request.
	url is a URL to the server or resource that the request targets.
	If url contains a protocol prefix, it is used. Otherwise, default_protocol is used.
*/
[[nodiscard]]
inline Request post(std::string_view const url, Protocol const default_protocol = Protocol::Http)
{
    return Request{RequestMethod::Post, url, default_protocol};
}

/*
	Creates a PUT request.
	url is a URL to the server or resource that the request targets.
	If url contains a protocol prefix, it is used. Otherwise, default_protocol is used.
*/
[[nodiscard]]
inline Request put(std::string_view const url, Protocol const default_protocol = Protocol::Http)
{
    return Request{RequestMethod::Put, url, default_protocol};
}

/*
	Creates a http request.
	Can be used to do the same things as get() and post(), but with more method options.
	If url contains a protocol prefix, it is used. Otherwise, default_protocol is used.
*/
[[nodiscard]]
inline Request make_request(
        RequestMethod const method,
        std::string_view const url,
        Protocol const default_protocol = Protocol::Http
)
{
    return Request{method, url, default_protocol};
}

} // namespace http_client

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_REQUEST_HPP
