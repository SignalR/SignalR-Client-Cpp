// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "pplx/pplxtasks.h"
#include "cpprest/details/basic_types.h"
#include "cpprest/http_client.h"
#include "web_request_factory.h"


namespace signalr
{
    namespace http_sender
    {
        pplx::task<utility::string_t> get(web_request_factory& request_factory, const web::uri& url,
            const std::unordered_map<utility::string_t, utility::string_t>& headers,
            const web::http::client::http_client_config &client_config = web::http::client::http_client_config{});
    }
}