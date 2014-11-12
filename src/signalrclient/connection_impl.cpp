// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include <cpprest\asyncrt_utils.h>
#include "connection_impl.h"
#include "request_sender.h"

namespace signalr
{
    std::shared_ptr<connection_impl> connection_impl::create(const utility::string_t& url, const utility::string_t& querystring)
    {
        return connection_impl::create(url, querystring, std::make_unique<web_request_factory>(), std::make_unique<transport_factory>());
    }

    std::shared_ptr<connection_impl> connection_impl::create(const utility::string_t& url, const utility::string_t& querystring,
        std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory)
    {
        return std::shared_ptr<connection_impl>(new connection_impl(url, querystring, std::move(web_request_factory), std::move(transport_factory)));
    }

    connection_impl::connection_impl(const utility::string_t& url, const utility::string_t& querystring,
        std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory)
        : m_base_uri(url), m_querystring(querystring), m_web_request_factory(std::move(web_request_factory)), 
        m_transport_factory(std::move(transport_factory)), m_connection_state(std::move(connection_state::disconnected))
    { }

    pplx::task<void> connection_impl::start()
    {
        if (!change_state(connection_state::disconnected, connection_state::connecting))
        {
            throw std::runtime_error(utility::conversions::to_utf8string(
                _XPLATSTR("cannot start a connection that is not in the disconnected state")));
        }

        return pplx::task_from_result();
    }

    connection_state connection_impl::get_connection_state() const
    {
        return m_connection_state.load();
    }

    bool connection_impl::change_state(connection_state old_state, connection_state new_state)
    {
        connection_state expected_state{ old_state };

        if (!m_connection_state.compare_exchange_strong(expected_state, new_state, std::memory_order_seq_cst))
        {
            // TODO: add logging
            // TODO: invoke state_changed callback
            return false;
        }

        return true;
    }
}