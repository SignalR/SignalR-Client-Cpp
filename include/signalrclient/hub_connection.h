// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "_exports.h"
#include <memory>
#include <functional>
#include "pplx/pplxtasks.h"
#include "cpprest/json.h"
#include "connection_state.h"
#include "trace_level.h"
#include "log_writer.h"
#include "hub_proxy.h"

namespace signalr
{
    class hub_connection_impl;

    class hub_connection
    {
    public:
        SIGNALRCLIENT_API explicit hub_connection(const utility::string_t& url, const utility::string_t& query_string = U(""),
            trace_level trace_level = trace_level::all, std::shared_ptr<log_writer> log_writer = nullptr, bool use_default_url = true);

        SIGNALRCLIENT_API ~hub_connection();

        hub_connection(const hub_connection&) = delete;

        hub_connection& operator=(const hub_connection&) = delete;

        SIGNALRCLIENT_API pplx::task<void> __cdecl start();
        SIGNALRCLIENT_API pplx::task<void> __cdecl stop();

        SIGNALRCLIENT_API hub_proxy __cdecl create_hub_proxy(const utility::string_t& hub_name);

        SIGNALRCLIENT_API connection_state __cdecl get_connection_state() const;

        SIGNALRCLIENT_API void __cdecl set_reconnecting(const std::function<void __cdecl()>& reconnecting_callback);
        SIGNALRCLIENT_API void __cdecl set_reconnected(const std::function<void __cdecl()>& reconnected_callback);
        SIGNALRCLIENT_API void __cdecl set_disconnected(const std::function<void __cdecl()>& disconnected_callback);

        SIGNALRCLIENT_API void __cdecl set_headers(const std::unordered_map<utility::string_t, utility::string_t>& headers);

    private:
        std::shared_ptr<hub_connection_impl> m_pImpl;
        pplx::task<web::json::value> invoke_json(const utility::string_t& hub_name, const utility::string_t& method_name, const web::json::value& arguments,
            const std::function<void(const web::json::value&)>& on_progress);
    };
}