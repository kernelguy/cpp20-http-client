# c++20 internet client

c++20 internet client is an HTTP/HTTPS client library written in c++20. The library only consists of a single header and source file.

As of now, only gcc supports all of the c++20 features used in this library. Additionally, there are some c++20 features that are not used in the library because no compiler or standard library yet supports them. However the library will be updated over time as compilers start implementing more of c++20.


## Aims
* User friendly, functional design.
* An API that is hard to misuse.
* Library code follows c++ core guidelines.
* A single module file when build systems and compilers have good support for modules.
* Support for GET and POST requests, both unsecured and over TLS.
* Support for asynchronous requests.
* Support for Windows, Linux and MacOS.


## Simple "GET" request example
Note that the fmt library is not a dependency of this library, it's just to simplify the example.
```cpp
#include <cpp20_internet_client.hpp>
#include <fmt/format.h>

using namespace internet_client;

auto main() -> int {
    try {
        auto const response = http::get("https://www.google.com")
            .add_header({.name="HeaderName", .value="header value"})
            .send();
        fmt::print("Date from server: {}.\n", response.get_header_value("date").value_or("Unknown"));
        response.write_body_to_file("index.html");
    } 
    catch (errors::ConnectionFailed const& error) {
        fmt::print("The connection failed - \"{}\"\n", error.what());
    }
}
```

## Development status
The project is in its initial development and only a subset of the functionality has been implemented.
