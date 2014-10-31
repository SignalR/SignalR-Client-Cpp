// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "connection_impl.h"

namespace signalr
{
    connection_impl::connection_impl(const utility::string_t& url, const utility::string_t& querystring,
        web_request_factory& web_request_factory, transport_factory& transport_factory)
        : m_base_uri(url), m_querystring(querystring), m_web_request_factory(web_request_factory),
        m_transport_factory(transport_factory), m_connection_state(connection_state::disconnected)
    { }

    pplx::task<void> connection_impl::start()
    {
        return pplx::task_from_result();
    }

    connection_state connection_impl::get_connection_state() const
    {
        return m_connection_state;
    }
}