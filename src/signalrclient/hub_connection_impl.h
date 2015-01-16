// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <unordered_map>
#include <cpprest\basic_types.h>
#include "connection_impl.h"
#include "internal_hub_proxy.h"
#include "callback_manager.h"

namespace signalr
{
    // Note:
    // Factory methods and private constructors prevent from using this class incorrectly. Because this class
    // derives from `std::enable_shared_from_this` the instance has to be owned by a `std::shared_ptr` whenever
    // a member method calls `std::shared_from_this()` otherwise the behavior is undefined. Therefore constructors
    // are private to disallow creating instances directly and factory methods return `std::shared_ptr<connection_impl>`.
    class hub_connection_impl : public std::enable_shared_from_this<hub_connection_impl>
    {
    public:
        static std::shared_ptr<hub_connection_impl> create(const utility::string_t& url, const utility::string_t& query_string,
            trace_level trace_level, const std::shared_ptr<log_writer>& log_writer, std::unique_ptr<web_request_factory> web_request_factory,
            std::unique_ptr<transport_factory> transport_factory);

        hub_connection_impl(const hub_connection_impl&) = delete;
        hub_connection_impl& operator=(const hub_connection_impl&) = delete;

        std::shared_ptr<internal_hub_proxy> create_hub_proxy(const utility::string_t& hub_name);
        pplx::task<json::value> invoke_json(const utility::string_t& hub_name, const utility::string_t& method_name, const json::value& arguments,
            const std::function<void(const json::value&)>& on_progress = [](const json::value&){});
        pplx::task<void> invoke_void(const utility::string_t& hub_name, const utility::string_t& method_name, const json::value& arguments,
            const std::function<void(const json::value&)>& on_progress = [](const json::value&){});

        pplx::task<void> start();
        pplx::task<void> stop();

        connection_state get_connection_state() const;

    private:
        hub_connection_impl(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level,
            const std::shared_ptr<log_writer>& log_writer, std::unique_ptr<web_request_factory> web_request_factory,
            std::unique_ptr<transport_factory> transport_factory);

        //TODO: keep a copy or take from the connection?
        logger m_logger;

        std::shared_ptr<connection_impl> m_connection;
        std::unordered_map<utility::string_t, std::shared_ptr<internal_hub_proxy>> m_proxies;
        callback_manager m_callback_manager;

        void initialize();

        void process_message(const web::json::value& message);

        void invoke_hub_method(const utility::string_t& hub_name, const utility::string_t& method_name,
            const json::value& arguments, const utility::string_t& callback_id, std::function<void(const std::exception_ptr)> set_exception);
        bool invoke_callback(const web::json::value& message);
    };
}
