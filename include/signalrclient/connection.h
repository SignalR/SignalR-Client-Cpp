// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\http_client.h>
#include "_exports.h"
#include "transport_type.h"
#include "web_request_factory.h"
#include "transport_factory.h"

namespace signalr
{
    class connection_impl;

    class connection
    {
    public:
        connection(const utility::string_t& url, const utility::string_t& querystring = U(""));

        connection(const connection&) = delete;

        connection& operator=(const connection&) = delete;

        ~connection();

        SIGNALRCLIENT_API pplx::task<void> start();

    private:
        web::uri m_base_uri;
        utility::string_t m_querystring;

        // the order is important since we the factories are used to create and initialize the connection_impl instance
        web_request_factory m_web_request_factory;
        transport_factory m_transport_factory;

        connection_impl *m_pImpl;
    };
}