#ifndef CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_CLEANUP_HPP
#define CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_CLEANUP_HPP

#include <concepts>
#include <utility>

namespace http_client::utils {

/*
	Used to invoke a lambda at the end of a scope.
*/
template<std::invocable T>
class [[nodiscard]] Cleanup
{
public:
    [[nodiscard]] Cleanup(T&& callable)
            : callable_{std::forward<T>(callable)}
    {
    }

    Cleanup() = delete;

    ~Cleanup()
    {
        callable_();
    }

    Cleanup(Cleanup&&) noexcept = delete;
    Cleanup& operator=(Cleanup&&) noexcept = delete;

    Cleanup(Cleanup const&) = delete;
    Cleanup& operator=(Cleanup const&) = delete;

private:
    T callable_;
};

} // namespace http_client::utils

#endif //CPP20HTTPCLIENT_SOURCE_CPP20_HTTP_CLIENT_CPP_CLEANUP_HPP
