/**
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* \copyright   Copyright 2025 RSP Systems A/S. All rights reserved.
* \license     Mozilla Public License 2.0
* \author      steffen
*/
#ifndef CPP20HTTPCLIENT_INCLUDE_HTTP_CLIENT_STATUS_LINE_HPP
#define CPP20HTTPCLIENT_INCLUDE_HTTP_CLIENT_STATUS_LINE_HPP

#include "StatusCode.hpp"
#include "utils/template-helpers.hpp"
#include <string>

namespace http_client {

struct StatusLine
{
    std::string http_version;
    StatusCode status_code = StatusCode::Unknown;
    std::string status_message;

    [[nodiscard]]
    bool operator==(StatusLine const&) const noexcept = default;
};


} // namespace http_client

#endif //CPP20HTTPCLIENT_INCLUDE_HTTP_CLIENT_STATUS_LINE_HPP
