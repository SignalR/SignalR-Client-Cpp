// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <atomic>
#include <mutex>
#include "cpprest\http_client.h"
#include "signalrclient\trace_level.h"
#include "signalrclient\connection_state.h"
#include "web_request_factory.h"
#include "transport_factory.h"
#include "logger.h"
#include "negotiation_response.h"

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
            trace_level trace_level, const std::shared_ptr<log_writer>& log_writer);

        static std::shared_ptr<connection_impl> create(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level,
            const std::shared_ptr<log_writer>& log_writer, std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory);

        connection_impl(const connection_impl&) = delete;

        connection_impl& operator=(const connection_impl&) = delete;

        ~connection_impl();

        pplx::task<void> start();
        pplx::task<void> send(const utility::string_t &data);
        pplx::task<void> stop();

        connection_state get_connection_state() const;

        void set_message_received_string(const std::function<void(const utility::string_t&)>& message_received);
        void set_message_received_json(const std::function<void(const web::json::value&)>& message_received);
        void set_connection_data(const utility::string_t& connection_data);
        void set_headers(const std::unordered_map<utility::string_t, utility::string_t>& headers);

    private:
        web::uri m_base_url;
        utility::string_t m_query_string;
        std::atomic<connection_state> m_connection_state;
        logger m_logger;
        std::shared_ptr<transport> m_transport;
        std::unique_ptr<web_request_factory> m_web_request_factory;
        std::unique_ptr<transport_factory> m_transport_factory;

        std::function<void(const web::json::value&)> m_message_received;
        std::unordered_map<utility::string_t, utility::string_t> m_headers;

        pplx::cancellation_token_source m_disconnect_cts;
        std::mutex m_stop_lock;
        pplx::event m_start_completed_event;
        utility::string_t m_connection_token;
        utility::string_t m_connection_data;
        utility::string_t m_message_id;
        utility::string_t m_groups_token;

        connection_impl(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level, const std::shared_ptr<log_writer>& log_writer,
            std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory);

        pplx::task<std::shared_ptr<transport>> start_transport(negotiation_response negotiation_response);
        pplx::task<void> send_connect_request(const std::shared_ptr<transport>& transport, const utility::string_t& connection_token,
            const pplx::task_completion_event<void>& connect_request_tce);

        void process_response(const utility::string_t& response, const pplx::task_completion_event<void>& connect_request_tce);

        pplx::task<void> shutdown();

        bool change_state(connection_state old_state, connection_state new_state);
        connection_state change_state(connection_state new_state);
        void handle_connection_state_change(connection_state old_state, connection_state new_state);
        void invoke_message_received(const web::json::value& message);

        static utility::string_t translate_connection_state(connection_state state);
        void ensure_disconnected(const std::string& error_message);
    };
}
