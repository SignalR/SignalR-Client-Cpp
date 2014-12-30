// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <unordered_map>
#include <cpprest\basic_types.h>
#include "internal_hub_proxy.h"

namespace signalr
{
    class hub_connection_impl
    {
    public:
        hub_connection_impl(trace_level trace_level, const std::shared_ptr<log_writer>& log_writer);

        hub_connection_impl(const hub_connection_impl&) = delete;

        hub_connection_impl& operator=(const hub_connection_impl&) = delete;

        std::shared_ptr<internal_hub_proxy> create_hub_proxy(const utility::string_t& hub_name);

    private:
        //TODO: keep a copy or take from the connection?
        logger m_logger;

        std::unordered_map<utility::string_t, std::shared_ptr<internal_hub_proxy>> m_proxies;
    };
}
