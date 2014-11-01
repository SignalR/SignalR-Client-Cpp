// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <atomic>
#include <cpprest\http_client.h>
#include "signalrclient\web_request_factory.h"
#include "signalrclient\transport_factory.h"
#include "signalrclient\connection_state.h"

namespace signalr
{
    class connection_impl
    {
    public:
        // taking references to be able to inject factories for test purposes. The caller must 
        // make sure that actual instances outlive connection_impl
        connection_impl(const utility::string_t& url, const utility::string_t& querystring,
            web_request_factory& web_request_factory, transport_factory& transport_factory);

        connection_impl(const connection_impl&) = delete;

        connection_impl& operator=(const connection_impl&) = delete;

        pplx::task<void> start();

        connection_state get_connection_state() const;

    private:
        web::uri m_base_uri;
        utility::string_t m_querystring;
        std::atomic<connection_state> m_connection_state;

        web_request_factory &m_web_request_factory;
        transport_factory& m_transport_factory;

        bool change_state(connection_state old_state, connection_state new_state);
    };
}
