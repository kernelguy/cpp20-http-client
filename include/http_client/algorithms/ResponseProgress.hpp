#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_PROGRESS_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_PROGRESS_HPP

#include "ParsedHeadersInterface.hpp"

namespace http_client::algorithms {

class ResponseParser;


class ResponseProgressRaw
{
    friend class ResponseParser;

public:
    constexpr void stop() noexcept
    {
        is_stopped_ = true;
    }

    std::span<std::byte const> data;
    std::size_t new_data_start;

    constexpr ResponseProgressRaw(std::span<std::byte const> const p_data,
                                           std::size_t const p_new_data_start) noexcept
        : data{p_data},
          new_data_start{p_new_data_start}
    {
    }

private:
    bool is_stopped_{false};
};



class ResponseProgressHeaders : public algorithms::ParsedHeadersInterface
{
public:
    ResponseProgressRaw raw_progress;

    constexpr void stop() noexcept
    {
        raw_progress.stop();
    }

    [[nodiscard]]
    constexpr algorithms::ParsedResponse const &get_parsed_response() const noexcept override
    {
        return parsed_response_;
    }

    ResponseProgressHeaders(ResponseProgressRaw const p_raw_progress, algorithms::ParsedResponse const &parsed_response)
            :
            raw_progress{p_raw_progress}, parsed_response_{parsed_response}
    {}

    ResponseProgressHeaders() = delete;

    ResponseProgressHeaders(ResponseProgressHeaders const &) = delete;

    ResponseProgressHeaders &operator=(ResponseProgressHeaders const &) = delete;

    ResponseProgressHeaders(ResponseProgressHeaders &&) noexcept = delete;

    ResponseProgressHeaders &operator=(ResponseProgressHeaders &&) noexcept = delete;

private:
    algorithms::ParsedResponse const &parsed_response_;
};



class ResponseProgressBody : public algorithms::ParsedHeadersInterface
{
public:
    ResponseProgressRaw raw_progress;

    std::span<std::byte const> body_data_so_far;
    /*
        This may not have a value if the transfer encoding is chunked, in which
        case the full body length is not known ahead of time.
    */
    std::optional<std::size_t> total_expected_body_size;

    constexpr void stop() noexcept
    {
        raw_progress.stop();
    }

    [[nodiscard]]
    constexpr algorithms::ParsedResponse const &get_parsed_response() const noexcept override
    {
        return parsed_response_;
    }

    ResponseProgressBody(
            ResponseProgressRaw const p_raw_progress,
            algorithms::ParsedResponse const &parsed_response,
            std::span<std::byte const> const p_body_data_so_far,
            std::optional<std::size_t> const p_total_expected_body_size
    ) :
            raw_progress{p_raw_progress},
            body_data_so_far{p_body_data_so_far},
            total_expected_body_size{p_total_expected_body_size},
            parsed_response_{parsed_response}
    {}

    ResponseProgressBody() = delete;

    ResponseProgressBody(ResponseProgressBody const &) = delete;

    ResponseProgressBody &operator=(ResponseProgressBody const &) = delete;

    ResponseProgressBody(ResponseProgressBody &&) noexcept = delete;

    ResponseProgressBody &operator=(ResponseProgressBody &&) noexcept = delete;

private:
    algorithms::ParsedResponse const &parsed_response_;
};

} // namespace http_client

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_PROGRESS_HPP
