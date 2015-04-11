// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "signalrclient/hub_connection.h"
#include "hub_connection_impl.h"

namespace signalr
{
    hub_connection::hub_connection(const utility::string_t& url, const utility::string_t& query_string,
        trace_level trace_level, std::shared_ptr<log_writer> log_writer, bool use_default_url)
        : m_pImpl(hub_connection_impl::create(url, query_string, trace_level, std::move(log_writer), use_default_url))
    {}

    // Do NOT remove this destructor. Letting the compiler generate and inline the default dtor may lead to
    // undefinded behavior since we are using an incomplete type. More details here:  http://herbsutter.com/gotw/_100/
    hub_connection::~hub_connection() = default;

    pplx::task<void> hub_connection::start()
    {
        return m_pImpl->start();
    }

    pplx::task<void> hub_connection::stop()
    {
        return m_pImpl->stop();
    }

    hub_proxy hub_connection::create_hub_proxy(const utility::string_t& hub_name)
    {
        auto internal_proxy = m_pImpl->create_hub_proxy(hub_name);
        return hub_proxy(internal_proxy);
    }

    connection_state hub_connection::get_connection_state() const
    {
        return m_pImpl->get_connection_state();
    }

    void hub_connection::set_reconnecting(const std::function<void()>& reconnecting_callback)
    {
        m_pImpl->set_reconnecting(reconnecting_callback);
    }

    void hub_connection::set_reconnected(const std::function<void()>& reconnected_callback)
    {
        m_pImpl->set_reconnected(reconnected_callback);
    }

    void hub_connection::set_disconnected(const std::function<void()>& disconnected_callback)
    {
        m_pImpl->set_disconnected(disconnected_callback);
    }

    void hub_connection::set_headers(const std::unordered_map<utility::string_t, utility::string_t>& headers)
    {
        m_pImpl->set_headers(headers);
    }
}