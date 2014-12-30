// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "internal_hub_proxy.h"

namespace signalr
{
    internal_hub_proxy::internal_hub_proxy(const utility::string_t& hub_name, const logger& logger) :
        m_hub_name(hub_name), m_logger(logger)
    { }

    utility::string_t internal_hub_proxy::get_hub_name() const
    {
        return m_hub_name;
    }

    void internal_hub_proxy::on(const utility::string_t& event_name, action_handler handler)
    {
        if (event_name.length() == 0)
        {
            throw std::invalid_argument("event_name cannot be empty");
        }

        if (m_subscriptions.find(event_name) != m_subscriptions.end())
        {
            throw std::runtime_error(std::string("an action for this event has already been registered. event name: ")
                .append(utility::conversions::to_utf8string(event_name)));
        }

        // TODO: either needs to be thread safe or we should not allow adding handlers if the connection is not closed (or both)
        m_subscriptions.insert(std::pair<utility::string_t, action_handler> {event_name, handler});
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
}