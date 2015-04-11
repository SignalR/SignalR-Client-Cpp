// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once
#include <functional>
#include "cpprest/details/basic_types.h"
#include "cpprest/json.h"
#include "logger.h"
#include "case_insensitive_comparison_utils.h"

using namespace web;

namespace signalr
{
    class hub_connection_impl;

    class internal_hub_proxy
    {
    public:
        internal_hub_proxy(const std::weak_ptr<hub_connection_impl>& hub_connection, const utility::string_t& hub_name, const logger& logger);

        internal_hub_proxy(const internal_hub_proxy&) = delete;
        internal_hub_proxy& operator=(const internal_hub_proxy&) = delete;

        utility::string_t get_hub_name() const;

        void on(const utility::string_t& event_name, const std::function<void(const json::value &)>& handler);
        void invoke_event(const utility::string_t& event_name, const json::value& arguments);

        pplx::task<json::value> invoke_json(const utility::string_t& method_name, const json::value& arguments,
            const std::function<void(const json::value&)>& on_progress = [](const json::value&){});
        pplx::task<void> invoke_void(const utility::string_t& method_name, const json::value& arguments,
            const std::function<void(const json::value&)>& on_progress = [](const json::value&){});

    private:
        std::weak_ptr<hub_connection_impl> m_hub_connection;
        const utility::string_t m_hub_name;
        logger m_logger;

        std::unordered_map<utility::string_t, std::function<void(const json::value &)>, case_insensitive_hash, case_insensitive_equals> m_subscriptions;
    };
}