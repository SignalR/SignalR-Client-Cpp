// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "_exports.h"
#include <memory>
#include <functional>
#include "pplx\pplxtasks.h"
#include "connection_state.h"
#include "trace_level.h"
#include "log_writer.h"
#include "trace_log_writer.h"
#include <unordered_map>

namespace signalr
{
    class connection_impl;

    class connection
    {
        // TODO: consider making public or remove?
        typedef std::function<void(const utility::string_t&)> message_received;

    public:
        SIGNALRCLIENT_API explicit connection(const utility::string_t& url, const utility::string_t& query_string = U(""),
            trace_level trace_level = trace_level::all, std::shared_ptr<log_writer> log_writer = std::make_shared<trace_log_writer>());

        SIGNALRCLIENT_API ~connection();

        connection(const connection&) = delete;

        connection& operator=(const connection&) = delete;

        SIGNALRCLIENT_API pplx::task<void> start();

        SIGNALRCLIENT_API pplx::task<void> send(const utility::string_t& data);

        SIGNALRCLIENT_API void set_message_received(const message_received& message_received_callback);

        SIGNALRCLIENT_API void set_headers(const std::unordered_map<utility::string_t, utility::string_t>& headers);

        SIGNALRCLIENT_API pplx::task<void> stop();

        SIGNALRCLIENT_API connection_state get_connection_state() const;

    private:
        // The recommended smart pointer to use when doing pImpl is the `std::unique_ptr`. However
        // we are capturing the m_pImpl instance in the lambdas used by tasks which can outlive
        // the connection instance. Using `std::shared_ptr` guarantees that we won't be using
        // a deleted object if the task is run after the `connection` instance goes away.
        std::shared_ptr<connection_impl> m_pImpl;
    };
}