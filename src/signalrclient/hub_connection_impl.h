// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <unordered_map>
#include <cpprest\basic_types.h>
#include "connection_impl.h"
#include "internal_hub_proxy.h"

namespace signalr
{
    class hub_connection_impl
    {
    public:
        hub_connection_impl(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level,
            const std::shared_ptr<log_writer>& log_writer, std::unique_ptr<web_request_factory> web_request_factory,
            std::unique_ptr<transport_factory> transport_factory);

        hub_connection_impl(const hub_connection_impl&) = delete;

        hub_connection_impl& operator=(const hub_connection_impl&) = delete;

        std::shared_ptr<internal_hub_proxy> create_hub_proxy(const utility::string_t& hub_name);

        pplx::task<void> start();

        pplx::task<void> stop();

        connection_state get_connection_state() const;

    private:
        //TODO: keep a copy or take from the connection?
        logger m_logger;

        std::shared_ptr<connection_impl> m_connection;

        std::unordered_map<utility::string_t, std::shared_ptr<internal_hub_proxy>> m_proxies;
    };
}
