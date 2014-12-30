// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "hub_connection_impl.h"

namespace signalr
{
    hub_connection_impl::hub_connection_impl(trace_level trace_level, const std::shared_ptr<log_writer>& log_writer)
        : m_logger(log_writer, trace_level)
    { }

    std::shared_ptr<internal_hub_proxy> hub_connection_impl::create_hub_proxy(const utility::string_t& hub_name)
    {
        if (hub_name.length() == 0)
        {
            throw std::invalid_argument("hub name cannot be empty");
        }

        // TODO: throw if connection not in the disconnected state

        auto iter = m_proxies.find(hub_name);
        if (iter != m_proxies.end())
        {
            return iter->second;
        }

        auto proxy = std::make_shared<internal_hub_proxy>(hub_name, m_logger);
        m_proxies.insert(std::make_pair(hub_name, proxy));
        return proxy;
    }
}