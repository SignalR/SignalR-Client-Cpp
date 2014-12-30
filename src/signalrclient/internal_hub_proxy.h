// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once
#include <functional>
#include <cpprest\basic_types.h>
#include <cpprest\json.h>
#include "logger.h"

using namespace web;

namespace signalr
{
    class internal_hub_proxy
    {
        // TODO: consider making visible outside
    private:
        typedef std::function<void(const json::value &)> action_handler;

    public:
        // TODO: `logger` or just `weak_ptr (hub_)connection` or both (e.g. if we want to log after connection is gone)?
        internal_hub_proxy(const utility::string_t& hub_name, const logger& logger);

        internal_hub_proxy(const internal_hub_proxy&) = delete;

        internal_hub_proxy& operator=(const internal_hub_proxy&) = delete;

        void on(const utility::string_t& event_name, action_handler handler);

        void invoke_event(const utility::string_t& event_name, const json::value& arguments);

        utility::string_t get_hub_name() const;

    private:
        const utility::string_t m_hub_name;
        logger m_logger;

        std::unordered_map<utility::string_t, action_handler> m_subscriptions;
    };
}