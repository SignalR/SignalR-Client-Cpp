// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "signalrclient\connection.h"
#include "signalrclient\transport_type.h"
#include "connection_impl.h"

namespace signalr
{
    connection::connection(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level, std::shared_ptr<log_writer> log_writer)
        : m_pImpl(connection_impl::create(url, query_string, trace_level, std::move(log_writer)))
    {}

    // Do NOT remove this destructor. Letting the compiler generate and inline the default dtor may lead to
    // undefinded behavior since we are using an incomplete type. More details here:  http://herbsutter.com/gotw/_100/
    connection::~connection() = default;

    pplx::task<void> connection::start()
    {
        return m_pImpl->start();
    }

    pplx::task<void> connection::send(const utility::string_t& data)
    {
        return m_pImpl->send(data);
    }

    void connection::set_message_received(const message_received& message_received_callback)
    {
        m_pImpl->set_message_received_string(message_received_callback);
    }

    void connection::set_headers(const std::unordered_map<utility::string_t, utility::string_t>& headers)
    {
        m_pImpl->set_headers(headers);
    }

    pplx::task<void> connection::stop()
    {
        return m_pImpl->stop();
    }

    connection_state connection::get_connection_state() const
    {
        return m_pImpl->get_connection_state();
    }
}