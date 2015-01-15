// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "_exports.h"
#include <memory>
#include <functional>
#include <ppltasks.h>
#include <cpprest\json.h>
#include "connection_state.h"
#include "trace_level.h"
#include "log_writer.h"
#include "trace_log_writer.h"
#include "hub_proxy.h"

namespace pplx = concurrency;

namespace signalr
{
    class hub_connection_impl;

    class hub_connection
    {
    public:
        SIGNALRCLIENT_API explicit hub_connection(const utility::string_t& url, const utility::string_t& query_string = U(""),
            trace_level trace_level = trace_level::all, std::shared_ptr<log_writer> log_writer = std::make_shared<trace_log_writer>());

        SIGNALRCLIENT_API ~hub_connection();

        SIGNALRCLIENT_API pplx::task<void> start();
        SIGNALRCLIENT_API pplx::task<void> stop();

        SIGNALRCLIENT_API hub_proxy create_hub_proxy(const utility::string_t& hub_name);

        SIGNALRCLIENT_API connection_state get_connection_state() const;

    private:
        std::shared_ptr<hub_connection_impl> m_pImpl;
        pplx::task<web::json::value> invoke_json(const utility::string_t& hub_name, const utility::string_t& method_name, const web::json::value& arguments,
            const std::function<void(const web::json::value&)>& on_progress);
    };
}