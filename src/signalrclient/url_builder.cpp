// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "constants.h"
#include <cpprest\http_client.h>

namespace signalr
{
    namespace url_builder
    {
        web::uri_builder build_uri(const web::uri &base_url, const utility::string_t &command, const utility::string_t &query_string)
        {
            return web::uri_builder(base_url)
                .append_path(command)
                .append_query(_XPLATSTR("clientProtocol"), PROTOCOL)
                .append_query(query_string);
        }

        web::uri build_negotiate(const web::uri &base_url, const utility::string_t &query_string)
        {
            return build_uri(base_url, _XPLATSTR("negotiate"), query_string).to_uri();
        }
    }
}