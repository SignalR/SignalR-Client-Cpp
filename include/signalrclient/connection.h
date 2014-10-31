// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\http_client.h>
#include "_exports.h"
#include "transport_type.h"
#include "web_request_factory.h"
#include "transport_factory.h"
#include "connection_state.h"

namespace signalr
{
    class connection_impl;

    class connection
    {
    public:
        explicit connection(const utility::string_t& url, const utility::string_t& querystring = U(""));

        connection(const connection&) = delete;

        connection& operator=(const connection&) = delete;

        ~connection();

        SIGNALRCLIENT_API pplx::task<void> start();

        SIGNALRCLIENT_API connection_state get_connection_state() const;

    private:
        web_request_factory m_web_request_factory;
        transport_factory m_transport_factory;

        connection_impl *m_pImpl;
    };
}