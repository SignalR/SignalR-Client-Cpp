// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\http_client.h>
#include "_exports.h"
#include "transport_type.h"
#include "web_request.h"

namespace signalr
{
    template<typename T = web_request>
    class connection_T
    {
    private:
        web::uri m_base_uri;
        utility::string_t m_querystring;

    public:
        connection_T(const utility::string_t& url, const utility::string_t& querystring = U(""))
            : m_base_uri(url), m_querystring(querystring)
        {}

        SIGNALRCLIENT_API pplx::task<void> start();
        SIGNALRCLIENT_API pplx::task<void> start(transport_type transport);
    };

    using connection = connection_T<>;
}