// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include <cpprest\asyncrt_utils.h>
#include "connection_impl.h"
#include "request_sender.h"

namespace signalr
{
    std::shared_ptr<connection_impl> connection_impl::create(const utility::string_t& url, const utility::string_t& querystring, 
        trace_level trace_level, std::shared_ptr<log_writer> log_writer)
    {
        return connection_impl::create(url, querystring, trace_level, log_writer, std::make_unique<web_request_factory>(), std::make_unique<transport_factory>());
    }

    std::shared_ptr<connection_impl> connection_impl::create(const utility::string_t& url, const utility::string_t& querystring, trace_level trace_level, 
        std::shared_ptr<log_writer> log_writer, std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory)
    {
        return std::shared_ptr<connection_impl>(new connection_impl(url, querystring, trace_level, log_writer, std::move(web_request_factory), std::move(transport_factory)));
    }

    connection_impl::connection_impl(const utility::string_t& url, const utility::string_t& querystring, trace_level trace_level, std::shared_ptr<log_writer> log_writer,
        std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory)
        : m_base_uri(url), m_querystring(querystring), m_logger(log_writer, trace_level), m_web_request_factory(std::move(web_request_factory)), 
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

    logger connection_impl::get_logger() const
    {
        return m_logger;
    }

    bool connection_impl::change_state(connection_state old_state, connection_state new_state)
    {
        connection_state expected_state{ old_state };

        if (m_connection_state.compare_exchange_strong(expected_state, new_state, std::memory_order_seq_cst))
        {
            m_logger.log(
                trace_level::state_changes,
                utility::string_t(_XPLATSTR("state changed: "))
                .append(translate_connection_state(old_state))
                .append(_XPLATSTR(" -> "))
                .append(translate_connection_state(new_state)));

            // TODO: invoke state_changed callback

            return true;
        }

        return false;
    }

    utility::string_t connection_impl::translate_connection_state(connection_state state)
    {
        switch (state)
        {
        case connection_state::connecting:
            return _XPLATSTR("connecting");
        case connection_state::connected:
            return _XPLATSTR("connected");
        case connection_state::reconnecting:
            return _XPLATSTR("reconnecting");
        case connection_state::disconnected:
            return _XPLATSTR("disconnected");
        default:
            _ASSERTE(false);
            return _XPLATSTR("(unknown)");
        }
    }
}