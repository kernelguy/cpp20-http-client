/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_PARSER_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_PARSER_HPP

#include "cpp20_http_client.hpp"

/*
	Separate, testable module that parses a http response.
	It has support for optional response progress callbacks.
*/
class ResponseParser {
public:
	/*
		Parses a new packet of data from the HTTP response.
		If it reached the end of the response, the parsed result is returned.
	*/
	[[nodiscard]]
	std::optional<http_client::algorithms::ParsedResponse> parse_new_data(std::span<std::byte const> const data) {
		if (is_done_) {
			return {};
		}

		auto const new_data_start = buffer_.size();

		http_client::utils::append_to_vector(buffer_, data);

		if (callbacks_ && (*callbacks_)->handle_raw_progress) {
			auto raw_progress = http_client::ResponseProgressRaw{buffer_, new_data_start};
			(*callbacks_)->handle_raw_progress(raw_progress);
			if (raw_progress.is_stopped_) {
				finish_();
			}
		}

		if (!is_done_ && result_.headers_string.empty()) {
			try_parse_headers_(new_data_start);
		}

		if (!is_done_ && !result_.headers_string.empty()) {
			if (chunky_body_parser_) {
				parse_new_chunky_body_data_(new_data_start);
			}
			else {
				parse_new_regular_body_data_(new_data_start);
			}
		}
		if (is_done_) {
			return std::move(result_);
		}
		return {};
	}

	ResponseParser() = default;
	ResponseParser(http_client::algorithms::ResponseCallbacks& callbacks) :
		callbacks_{&callbacks}
	{}

private:
	void finish_() {
		is_done_ = true;
		if (callbacks_ && (*callbacks_)->handle_stop) {
			(*callbacks_)->handle_stop();
		}
	}

	void try_parse_headers_(std::size_t const new_data_start) {
		if (auto const headers_string = try_extract_headers_string_(new_data_start))
		{
			result_.headers_string = *headers_string;

			auto status_line_end = result_.headers_string.find_first_of("\r\n");
			if (status_line_end == std::string_view::npos) {
				status_line_end = result_.headers_string.size();
			}

			result_.status_line = http_client::algorithms::parse_status_line(
				std::string_view{result_.headers_string}.substr(0, status_line_end)
			);

			if (result_.headers_string.size() > status_line_end) {
				result_.headers = http_client::algorithms::parse_headers_string(
					std::string_view{result_.headers_string}.substr(status_line_end)
				);
			}

			if (callbacks_ && (*callbacks_)->handle_headers) {
				auto progress_headers = http_client::ResponseProgressHeaders{http_client::ResponseProgressRaw{buffer_, new_data_start}, result_};
				(*callbacks_)->handle_headers(progress_headers);
				if (progress_headers.raw_progress.is_stopped_) {
					finish_();
				}
			}

			if (auto const body_size_try = get_body_size_()) {
				body_size_ = *body_size_try;
			}
			else if (auto const transfer_encoding = http_client::algorithms::find_header_by_name(result_.headers, "transfer-encoding");
				transfer_encoding && transfer_encoding->value == "chunked")
			{
				chunky_body_parser_ = ChunkyBodyParser{};
			}
		}
	}
	[[nodiscard]]
	std::optional<std::string_view> try_extract_headers_string_(std::size_t const new_data_start) {
		// '\n' line endings are not conformant with the HTTP standard.
		for (std::string_view const empty_line : {"\r\n\r\n", "\n\n"})
		{
			auto const find_start = new_data_start >= empty_line.length() - 1 ? new_data_start - (empty_line.length() - 1) : std::size_t{};

			auto const string_view_to_search = http_client::utils::data_to_string(std::span{buffer_});

			if (auto const position = string_view_to_search.find(empty_line, find_start);
				position != std::string_view::npos)
			{
				body_start_ = position + empty_line.length();
				return string_view_to_search.substr(0, position);
			}
		}
		return {};
	}
	[[nodiscard]]
	std::optional<std::size_t> get_body_size_() const {
		if (auto const content_length_string =
				http_client::algorithms::find_header_by_name(result_.headers, "content-length"))
		{
			if (auto const parse_result =
					http_client::utils::string_to_integral<std::size_t>(content_length_string->value))
			{
				return *parse_result;
			}
		}
		return {};
	}

	void parse_new_chunky_body_data_(std::size_t const new_data_start) {
		// May need to add an offset if this packet is
		// where the headers end and the body starts.
		auto const body_parse_start = std::max(new_data_start, body_start_);
		if (auto body = chunky_body_parser_->parse_new_data(std::span{buffer_}.subspan(body_parse_start)))
		{
			result_.body_data = *std::move(body);

			if (callbacks_ && (*callbacks_)->handle_body_progress) {
				auto body_progress = http_client::ResponseProgressBody{
					http_client::ResponseProgressRaw{buffer_, new_data_start},
					result_,
					result_.body_data, {}
				};
				(*callbacks_)->handle_body_progress(body_progress);
			}

			finish_();
		}
		else if (callbacks_ && (*callbacks_)->handle_body_progress) {
			auto body_progress = http_client::ResponseProgressBody{
				http_client::ResponseProgressRaw{buffer_, new_data_start},
				result_,
				chunky_body_parser_->get_result_so_far(), {}
			};
			(*callbacks_)->handle_body_progress(body_progress);
			if (body_progress.raw_progress.is_stopped_) {
				finish_();
			}
		}
	}

	void parse_new_regular_body_data_(std::size_t const new_data_start) {
		if (buffer_.size() >= body_start_ + body_size_) {
			auto const body_begin = buffer_.begin() + static_cast<std::ptrdiff_t>(body_start_);
			result_.body_data = http_client::utils::DataVector(body_begin, body_begin + static_cast<std::ptrdiff_t>(body_size_));

			if (callbacks_ && (*callbacks_)->handle_body_progress) {
				auto body_progress = http_client::ResponseProgressBody{
					http_client::ResponseProgressRaw{buffer_, new_data_start},
					result_,
					result_.body_data,
					body_size_
				};
				(*callbacks_)->handle_body_progress(body_progress);
			}

			finish_();
		}
		else if (callbacks_ && (*callbacks_)->handle_body_progress) {
			auto body_progress = http_client::ResponseProgressBody{
				http_client::ResponseProgressRaw{buffer_, new_data_start},
				result_,
				std::span{buffer_}.subspan(body_start_),
				body_size_
			};
			(*callbacks_)->handle_body_progress(body_progress);
			if (body_progress.raw_progress.is_stopped_) {
				finish_();
			}
		}
	}

	http_client::utils::DataVector buffer_;

	http_client::algorithms::ParsedResponse result_;
	bool is_done_{false};

	std::size_t body_start_{};
	std::size_t body_size_{};

	std::optional<ChunkyBodyParser> chunky_body_parser_;

	std::optional<http_client::algorithms::ResponseCallbacks*> callbacks_;
};

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_PARSER_HPP
