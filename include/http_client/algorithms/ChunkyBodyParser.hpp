#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_CHUNKY_BODY_PARSER_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_CHUNKY_BODY_PARSER_HPP

#include "../utils/template-helpers.hpp"
#include "../exceptions.hpp"

namespace http_client::algorithms {

class ChunkyBodyParser
{
public:
    [[nodiscard]]
    std::optional<utils::DataVector> parse_new_data(std::span<std::byte const> const new_data)
    {
        if (has_returned_result_) {
            return {};
        }
        if (is_finished_) {
            has_returned_result_ = true;
            return std::move(result_);
        }

        auto cursor = start_parse_offset_;

        while (true) {
            if (cursor >= new_data.size()) {
                start_parse_offset_ = cursor - new_data.size();
                return {};
            }
            if (auto const cursor_offset = parse_next_part_(new_data.subspan(cursor))) {
                cursor += cursor_offset;
            } else {
                has_returned_result_ = true;
                return std::move(result_);
            }
        }
    }

    [[nodiscard]]
    std::span<std::byte const> get_result_so_far() const
    {
        return result_;
    }

private:
    static constexpr auto newline = std::string_view{"\r\n"};

    /*
        "part" refers to a separately parsed unit of data.
        This partitioning makes the parsing algorithm simpler.
        Returns the position where the part ended.
        It may be past the end of the part.
    */
    [[nodiscard]]
    std::size_t parse_next_part_(std::span<std::byte const> const new_data)
    {
        if (chunk_size_left_) {
            return parse_chunk_body_part_(new_data);
        } else return parse_chunk_separator_part_(new_data);
    }

    [[nodiscard]]
    std::size_t parse_chunk_body_part_(std::span<std::byte const> const new_data)
    {
        if (chunk_size_left_ > new_data.size()) {
            chunk_size_left_ -= new_data.size();
            utils::append_to_vector(result_, new_data);
            return new_data.size();
        } else {
            utils::append_to_vector(result_, new_data.first(chunk_size_left_));

            // After each chunk, there is a \r\n and then the size of the next chunk.
            // We skip the \r\n so the next part starts at the size number.
            auto const part_end = chunk_size_left_ + newline.size();
            chunk_size_left_ = 0;
            return part_end;
        }
    }

    [[nodiscard]]
    std::size_t parse_chunk_separator_part_(std::span<std::byte const> const new_data)
    {
        auto const data_string = utils::data_to_string(new_data);

        auto const first_newline_character_pos = data_string.find(newline[0]);

        if (first_newline_character_pos == std::string_view::npos) {
            chunk_size_string_buffer_ += data_string;
            return new_data.size();
        } else if (chunk_size_string_buffer_.empty()) {
            parse_chunk_size_left_(data_string.substr(0, first_newline_character_pos));
        } else {
            chunk_size_string_buffer_ += data_string.substr(0, first_newline_character_pos);
            parse_chunk_size_left_(chunk_size_string_buffer_);
            chunk_size_string_buffer_.clear();
        }

        if (chunk_size_left_ == 0) {
            is_finished_ = true;
            return 0;
        }

        return first_newline_character_pos + newline.size();
    }

    void parse_chunk_size_left_(std::string_view const string)
    {
        // hexadecimal
        if (auto const result = utils::string_to_integral<std::size_t>(string, 16)) {
            chunk_size_left_ = *result;
        } else throw errors::ResponseParsingFailed{"Failed parsing http body chunk size."};
    }

    utils::DataVector result_;

    bool is_finished_{false};
    bool has_returned_result_{false};

    std::size_t start_parse_offset_{};

    std::string chunk_size_string_buffer_;
    std::size_t chunk_size_left_{};
};

} // namespace http_client

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_CHUNKY_BODY_PARSER_HPP
