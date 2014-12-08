// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <atomic>
#include <cpprest\http_client.h>
#include "signalrclient\trace_level.h"
#include "signalrclient\connection_state.h"
#include "web_request_factory.h"
#include "transport_factory.h"
#include "logger.h"

namespace signalr
{
    // Note:
    // Factory methods and private constructors prevent from using this class incorrectly. Because this class
    // derives from `std::enable_shared_from_this` the instance has to be owned by a `std::shared_ptr` whenever
    // a member method calls `std::shared_from_this()` otherwise the behavior is undefined. Therefore constructors
    // are private to disallow creating instances directly and factory methods return `std::shared_ptr<connection_impl>`.
    class connection_impl : public std::enable_shared_from_this<connection_impl>
    {
    public:
        static std::shared_ptr<connection_impl> create(const utility::string_t& url, const utility::string_t& query_string,
            trace_level trace_level, std::shared_ptr<log_writer> log_writer);

        static std::shared_ptr<connection_impl> create(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level,
            std::shared_ptr<log_writer> log_writer, std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory);

        connection_impl(const connection_impl&) = delete;

        connection_impl& operator=(const connection_impl&) = delete;

        pplx::task<void> start();
        pplx::task<void> send(utility::string_t data);

        connection_state get_connection_state() const;

        void set_message_received(const std::function<void(const utility::string_t&)>& message_received);

    private:
        web::uri m_base_url;
        utility::string_t m_query_string;
        std::atomic<connection_state> m_connection_state;
        logger m_logger;
        std::shared_ptr<transport> m_transport;
        std::unique_ptr<web_request_factory> m_web_request_factory;
        std::unique_ptr<transport_factory> m_transport_factory;

        std::function<void(const utility::string_t&)> m_message_received;

        pplx::task_completion_event<void> m_connect_request_tce;
        utility::string_t m_connection_token;

        connection_impl(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level, std::shared_ptr<log_writer> log_writer,
            std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory);

        pplx::task<void> send_connect_request(const utility::string_t& connection_token);

        void process_response(const utility::string_t& response);

        bool change_state(connection_state old_state, connection_state new_state);
        static utility::string_t translate_connection_state(connection_state state);
    };
}
