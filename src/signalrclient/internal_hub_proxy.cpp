// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "internal_hub_proxy.h"
#include "hub_connection_impl.h"

namespace signalr
{
    internal_hub_proxy::internal_hub_proxy(const std::weak_ptr<hub_connection_impl>& hub_connection, const utility::string_t& hub_name, const logger& logger)
        : m_hub_connection(hub_connection), m_hub_name(hub_name), m_logger(logger)
    { }

    utility::string_t internal_hub_proxy::get_hub_name() const
    {
        return m_hub_name;
    }

    void internal_hub_proxy::on(const utility::string_t& event_name, const std::function<void(const json::value &)>& handler)
    {
        if (event_name.length() == 0)
        {
            throw std::invalid_argument("event_name cannot be empty");
        }

        auto connection = m_hub_connection.lock();
        if (connection && connection->get_connection_state() != connection_state::disconnected)
        {
            throw std::runtime_error("can't register a handler if the connection is in a disconnected state");
        }

        if (m_subscriptions.find(event_name) != m_subscriptions.end())
        {
            throw std::runtime_error(std::string("an action for this event has already been registered. event name: ")
                .append(utility::conversions::to_utf8string(event_name)));
        }

        m_subscriptions.insert(std::pair<utility::string_t, std::function<void(const json::value &)>> {event_name, handler});
    }

    void internal_hub_proxy::invoke_event(const utility::string_t& event_name, const json::value& arguments)
    {
        auto handler = m_subscriptions.find(event_name);
        if (handler != m_subscriptions.end())
        {
            handler->second(arguments);
        }
        else
        {
            m_logger.log(trace_level::info,
                utility::string_t(_XPLATSTR("no handler found for event. hub name: "))
                .append(m_hub_name).append(_XPLATSTR(", event name: ")).append(event_name));
        }
    }

    pplx::task<json::value> internal_hub_proxy::invoke_json(const utility::string_t& method_name, const json::value& arguments,
        const std::function<void(const json::value&)>& on_progress)
    {
        auto connection = m_hub_connection.lock();
        if (!connection)
        {
            return pplx::task_from_exception<json::value>(
                std::runtime_error("the connection for which this hub proxy was created is no longer valid - it was either destroyed or went out of scope"));
        }

        return connection->invoke_json(get_hub_name(), method_name, arguments, on_progress);
    }

    pplx::task<void> internal_hub_proxy::invoke_void(const utility::string_t& method_name, const json::value& arguments,
        const std::function<void(const json::value&)>& on_progress)
    {
        auto connection = m_hub_connection.lock();
        if (!connection)
        {
            return pplx::task_from_exception<void>(
                std::runtime_error("the connection for which this hub proxy was created is no longer valid - it was either destroyed or went out of scope"));
        }

        return connection->invoke_void(get_hub_name(), method_name, arguments, on_progress);
    }
}