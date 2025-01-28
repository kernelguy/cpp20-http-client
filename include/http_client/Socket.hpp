
#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_SOCKET_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_SOCKET_HPP

#include <span>
#include <memory>
#include <string_view>
#include "Protocol.hpp"
#include "utils/template-helpers.hpp"

namespace http_client {

/*
	This type is used by the Socket class to signify that
	the peer closed the connection during a read call.
*/
struct ConnectionClosed
{
};

/*
	An abstraction on top of low level socket and TLS encryption APIs.
	Marking a Socket as const only means it won't be moved from or move assigned to.
*/
class Socket
{
public:
    /*
        Sends data to the peer through the socket.
    */
    void write(std::span<std::byte const> data) const;

    /*
        Sends a string to the peer through the socket.
        This function takes a basic_string_view, think about
        whether you want it to be null terminated or not.
    */
    void write(std::string_view const string_view) const
    {
        write(utils::string_to_data<std::byte>(string_view));
    }

    /*
        Receives data from the socket and reads it into a buffer.
        This function blocks until there is some data available.
        The data that was read may be smaller than the buffer.
        The function either returns the number of bytes that were read
        or a ConnectionClosed value if the peer closed the connection.
    */
    [[nodiscard("The result is important as it contains the size that was actually read.")]]
    std::variant <ConnectionClosed, std::size_t> read(std::span <std::byte> buffer) const;

    /*
        Receives data from the socket.
        This function blocks until there is some data available.
        The function either returns the buffer that was read
        or a ConnectionClosed value if the peer closed the connection.
        The returned DataVector may be smaller than what was requested.
    */
    [[nodiscard]]
    auto read(std::size_t const number_of_bytes = 512) const
    -> std::variant <ConnectionClosed, utils::DataVector>
    {
        auto result = utils::DataVector(number_of_bytes);
        if (auto const read_result = read(result); std::holds_alternative<std::size_t>(read_result)) {
            result.resize(std::get<std::size_t>(read_result));
            return result;
        }
        return ConnectionClosed{};
    }

    /*
        Reads any available data from the socket into a buffer.
        This function is nonblocking, and may return std::size_t{} if
        there was no data available. The function either returns the number
        of bytes that were read or a ConnectionClosed value if the peer
        closed the connection.
    */
    [[nodiscard("The result is important as it contains the size that was actually read.")]]
    std::variant <ConnectionClosed, std::size_t> read_available(std::span <std::byte> buffer) const;

    /*
        Reads any available data from the socket into a buffer.
        This function is nonblocking, and may return an empty vector if
        there was no data available. The function either returns a utils::DataVector
        of the data that was read or a ConnectionClosed value if the peer
        closed the connection.
    */
    template<std::size_t read_buffer_size = 512>
    [[nodiscard]]
    std::variant <ConnectionClosed, utils::DataVector> read_available() const
    {
        auto buffer = utils::DataVector(read_buffer_size);
        auto read_offset = std::size_t{};

        while (true) {
            if (auto const read_result = read_available(
                        std::span{buffer.data() + read_offset, read_buffer_size}
                ); std::holds_alternative<std::size_t>(read_result)) {
                if (auto const bytes_read = std::get<std::size_t>(read_result)) {
                    read_offset += bytes_read;
                    buffer.resize(read_offset + read_buffer_size);
                }
                else {
                    return buffer;
                }
            }
            else {
                return ConnectionClosed{};
            }
        }
    }

    Socket() = delete;

    ~Socket(); // = default in .cpp

    Socket(Socket &&) noexcept; // = default in .cpp
    Socket &operator=(Socket &&) noexcept; // = default in .cpp

    Socket(Socket const &) = delete;

    Socket &operator=(Socket const &) = delete;

private:
    class Implementation;

    std::unique_ptr <Implementation> implementation_;

    Socket(std::string_view server, Port port, bool is_tls_encrypted);

    friend Socket open_socket(std::string_view, Port, bool);
};

/*
	Opens a socket to a server through a port.
	If port is 443 OR is_tls_encrypted is true, TLS encryption is used.
	Otherwise it is unencrypted.
*/
[[nodiscard]]
inline Socket open_socket(std::string_view const server, Port const port, bool const is_tls_encrypted = false)
{
    return Socket{server, port, is_tls_encrypted};
}

} // namespace http_client

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_SOCKET_HPP
