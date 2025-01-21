
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// \copyright   Copyright (c) 2025 RSP Systems A/S. All rights reserved.
// \license     Mozilla Public License 2.0
// \author      Steffen Brummer

#ifndef CPP20HTTPCLIENT_HEADER_HPP
#define CPP20HTTPCLIENT_HEADER_HPP

#include <string>
#include "utils/template-helpers.hpp"

namespace http_client {

struct Header;

/*
	Represents a HTTP header whose data was copied from somewhere at some point.
	It consists of std::string objects instead of std::string_view.
*/
struct HeaderCopy
{
    std::string name, value;

    [[nodiscard]]
    inline explicit operator Header() const;
};

/*
	Represents a HTTP header whose data is not owned by this object.
	It consists of std::string_view objects instead of std::string.
*/
struct Header
{
    std::string_view name, value;

    [[nodiscard]]
    explicit operator HeaderCopy() const
    {
        return HeaderCopy{
                .name = std::string{name},
                .value = std::string{value},
        };
    }
};

HeaderCopy::operator Header() const
{
    return Header{
            .name = std::string_view{name},
            .value = std::string_view{value},
    };
}

template<typename T>
concept IsHeader = utils::IsAnyOf<T, HeaderCopy, Header>;

/*
	Compares two headers, taking into account case insensitivity.
*/
[[nodiscard]]
bool operator==(IsHeader auto const &lhs, IsHeader auto const &rhs)
{
    return lhs.value == rhs.value && utils::equal_ascii_case_insensitive(lhs.name, rhs.name);
}

} // namespace http_client

#endif //CPP20HTTPCLIENT_HEADER_HPP
