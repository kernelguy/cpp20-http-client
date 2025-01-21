#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_HPP

#include <chrono>
#include "algorithms/ParsedHeadersInterface.hpp"

namespace http_client {

/*
	Represents the response of a HTTP request.
*/
class Response : public algorithms::ParsedHeadersInterface
{
public:
    [[nodiscard]]
    constexpr algorithms::ParsedResponse const& get_parsed_response() const noexcept override
    {
        return parsed_response_;
    }

    /*
        Returns the body of the response.
        The returned std::span shall not outlive this Response object.
    */
    [[nodiscard]]
    std::span<std::byte const> get_body() const
    {
        return parsed_response_.body_data;
    }

    /*
        Returns the body of the response as a string.
        The returned std::string_view shall not outlive this Response object.
    */
    [[nodiscard]]
    std::string_view get_body_string() const
    {
        return utils::data_to_string(get_body());
    }

    [[nodiscard]]
    std::string_view get_url() const
    {
        return url_;
    }

    /*
        Return the total time as an std::chrono::duration<double, std::milli> object thus in µs (microseconds).
        If you wish to convert it to another unit, use: std::chrono::duration_cast<>().
    */
    [[nodiscard]]
    std::chrono::duration<double, std::milli> get_total_time() const
    {
        return total_time_;
    }

    // std::future requires default constructible on MSVC... Because of ABI stability.
    Response() = default;

    Response(Response const &) = delete;

    Response &operator=(Response const &) = delete;

    Response(Response &&) noexcept = default;

    Response &operator=(Response &&) noexcept = default;

    Response(algorithms::ParsedResponse &&parsed_response, std::string &&url,
             const std::chrono::duration<double, std::milli> &&total_time)
        : parsed_response_{std::move(parsed_response)},
          url_{std::move(url)},
          total_time_{total_time}
    {}

private:
    algorithms::ParsedResponse parsed_response_;
    std::string url_;
    std::chrono::duration<double, std::milli> total_time_{};
};

} // namespace http_client

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_HPP
