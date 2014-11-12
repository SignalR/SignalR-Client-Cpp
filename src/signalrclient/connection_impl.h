// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <atomic>
#include <cpprest\http_client.h>
#include "signalrclient\trace_level.h"
#include "signalrclient\connection_state.h"
#include "web_request_factory.h"
#include "transport_factory.h"
#include "logger.h"

namespace signalr
{
    class connection_impl
    {
    public:
        connection_impl(const utility::string_t& url, const utility::string_t& querystring);

        connection_impl(const utility::string_t& url, const utility::string_t& querystring,
            std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory);

        connection_impl(const connection_impl&) = delete;

        connection_impl& operator=(const connection_impl&) = delete;

        pplx::task<void> start();

        connection_state get_connection_state() const;

    private:
        web::uri m_base_uri;
        utility::string_t m_querystring;
        std::atomic<connection_state> m_connection_state;

        std::unique_ptr<web_request_factory> m_web_request_factory;
        std::unique_ptr<transport_factory> m_transport_factory;

        bool change_state(connection_state old_state, connection_state new_state);
    };
}
