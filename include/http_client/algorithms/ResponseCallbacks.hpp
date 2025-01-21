#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_CALLBACKS_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_CALLBACKS_HPP

#include <functional>
#include "../Response.hpp"
#include "ResponseProgress.hpp"

namespace http_client::algorithms {

struct ResponseCallbacks
{
    std::function<void(ResponseProgressRaw & )> handle_raw_progress;
    std::function<void(ResponseProgressHeaders & )> handle_headers;
    std::function<void(ResponseProgressBody & )> handle_body_progress;
    std::function<void(Response & )> handle_finish;
    std::function<void()> handle_stop;
};

} // namespace http_client::algorithms

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_RESPONSE_CALLBACKS_HPP
