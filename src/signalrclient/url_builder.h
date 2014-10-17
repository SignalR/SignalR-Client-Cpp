// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\http_client.h>
#include "signalrclient\transport_type.h"

namespace signalr
{
    namespace url_builder
    {
        web::uri build_negotiate(const web::uri &base_url, const utility::string_t &query_string);
        web::uri build_connect(const web::uri &base_url, transport_type transport,
            const utility::string_t &connection_token, const utility::string_t &query_string);
    }
}
