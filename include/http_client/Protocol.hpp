#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PROTOCOL_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PROTOCOL_HPP

namespace http_client {

using Port = int;

/*
	An enumeration of the transfer protocols that are supported by the library.
*/
enum class Protocol : Port
{
    Http = 80, Https = 443, Unknown = -1,
};

} // namespace http_client

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_PROTOCOL_HPP
