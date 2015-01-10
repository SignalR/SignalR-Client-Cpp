// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "hub_connection_impl.h"

namespace signalr
{
    // unnamed namespace makes it invisble outside this translation unit
    namespace
    {
        static std::function<void(const json::value&)> create_hub_invocation_callback(const logger& logger,
            std::function<void(const json::value&)> set_result, std::function<void(const std::exception_ptr e)> set_exception);
    }

    std::shared_ptr<hub_connection_impl> hub_connection_impl::create(const utility::string_t& url, const utility::string_t& query_string,
        trace_level trace_level, const std::shared_ptr<log_writer>& log_writer, std::unique_ptr<web_request_factory> web_request_factory,
        std::unique_ptr<transport_factory> transport_factory)
    {
        auto connection = std::shared_ptr<hub_connection_impl>(new hub_connection_impl(url, query_string, trace_level, log_writer,
            std::move(web_request_factory), std::move(transport_factory)));

        connection->initialize();

        return connection;
    }

    hub_connection_impl::hub_connection_impl(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level,
        const std::shared_ptr<log_writer>& log_writer, std::unique_ptr<web_request_factory> web_request_factory,
        std::unique_ptr<transport_factory> transport_factory)
        : m_connection(connection_impl::create(url, query_string, trace_level, log_writer, std::move(web_request_factory), std::move(transport_factory))),
        m_logger(log_writer, trace_level), m_callback_manager(json::value::parse(_XPLATSTR("{ \"E\" : \"connection went out of scope before invocation result was received\"}")))
    { }

    void hub_connection_impl::initialize()
    {
        auto this_hub_connection = shared_from_this();

        // weak_ptr prevents a circular dependency leading to memory leak and other problems
        auto weak_hub_connection = std::weak_ptr<hub_connection_impl>(this_hub_connection);

        m_connection->set_message_received_json([weak_hub_connection](const web::json::value& message)
        {
            auto connection = weak_hub_connection.lock();
            if (connection)
            {
                connection->process_message(message);
            }
        });
    }

    std::shared_ptr<internal_hub_proxy> hub_connection_impl::create_hub_proxy(const utility::string_t& hub_name)
    {
        if (hub_name.length() == 0)
        {
            throw std::invalid_argument("hub name cannot be empty");
        }

        if (get_connection_state() != connection_state::disconnected)
        {
            throw std::runtime_error("hub proxies cannot be created when the connection is not in the disconnected state");
        }

        auto iter = m_proxies.find(hub_name);
        if (iter != m_proxies.end())
        {
            return iter->second;
        }

        auto proxy = std::make_shared<internal_hub_proxy>(hub_name, m_logger);
        m_proxies.insert(std::make_pair(hub_name, proxy));
        return proxy;
    }

    pplx::task<void> hub_connection_impl::start()
    {
        if (m_proxies.size() > 0)
        {
            json::value connection_data;

            auto index = 0;
            for (auto kvp : m_proxies)
            {
                json::value hub;
                hub[_XPLATSTR("Name")] = json::value::string(kvp.first);

                connection_data[index++] = hub;
            }

            m_connection->set_connection_data(connection_data.serialize());
        }
        else
        {
            m_logger.log(trace_level::info, _XPLATSTR("no hub proxies exist for this hub connection"));
        }

        return m_connection->start();
    }

    pplx::task<void> hub_connection_impl::stop()
    {
        m_callback_manager.clear(json::value::parse(_XPLATSTR("{ \"E\" : \"connection was stopped before invocation result was received\"}")));
        return m_connection->stop();
    }

    void hub_connection_impl::process_message(const web::json::value& message)
    {
        if (message.is_object())
        {
            if (message.has_field(_XPLATSTR("I")))
            {
                auto callback_id = message.at(_XPLATSTR("I")).as_string();
                m_callback_manager.complete_callback(callback_id, message);
                return;
            }

            if (message.has_field(_XPLATSTR("H")) && message.has_field(_XPLATSTR("M")) && message.has_field(_XPLATSTR("A")))
            {
                auto hub_name = message.at(_XPLATSTR("H")).as_string();
                auto method = message.at(_XPLATSTR("M")).as_string();
                auto iter = m_proxies.find(hub_name);
                if (iter != m_proxies.end())
                {
                    iter->second->invoke_event(method, message.at(_XPLATSTR("A")));
                }
                else
                {
                    m_logger.log(trace_level::info,
                        utility::string_t(_XPLATSTR("no proxy found for hub invocation. hub: "))
                        .append(hub_name).append(_XPLATSTR(", method: ")).append(method));
                }

                return;
            }
        }

        m_logger.log(trace_level::info, utility::string_t(_XPLATSTR("non-hub message received and will be discarded. message: "))
            .append(message.serialize()));
    }

    pplx::task<json::value> hub_connection_impl::invoke_json(const utility::string_t& hub_name, const utility::string_t& method_name,
        const json::value& arguments)
    {
        _ASSERTE(arguments.is_array());

        pplx::task_completion_event<json::value> tce;

        const auto callback_id = m_callback_manager.register_callback(
            create_hub_invocation_callback(m_logger, [tce](const json::value& result) { tce.set(result); },
                [tce](const std::exception_ptr e) { tce.set_exception(e); }));

        invoke_hub_method(hub_name, method_name, arguments, callback_id,
            [tce](const std::exception_ptr e){tce.set_exception(e); });

        return pplx::create_task(tce);
    }

    pplx::task<void> hub_connection_impl::invoke_void(const utility::string_t& hub_name, const utility::string_t& method_name,
        const json::value& arguments)
    {
        _ASSERTE(arguments.is_array());

        pplx::task_completion_event<void> tce;

        const auto callback_id = m_callback_manager.register_callback(
            create_hub_invocation_callback(m_logger, [tce](const json::value&) { tce.set(); },
            [tce](const std::exception_ptr e){ tce.set_exception(e); }));

        invoke_hub_method(hub_name, method_name, arguments, callback_id,
            [tce](const std::exception_ptr e){tce.set_exception(e); });

        return pplx::create_task(tce);
    }

    void hub_connection_impl::invoke_hub_method(const utility::string_t& hub_name, const utility::string_t& method_name,
        const json::value& arguments, const utility::string_t& callback_id, std::function<void(const std::exception_ptr)> set_exception)
    {
        json::value request;
        request[_XPLATSTR("H")] = json::value::string(hub_name);
        request[_XPLATSTR("M")] = json::value::string(method_name);
        request[_XPLATSTR("A")] = arguments;
        request[_XPLATSTR("I")] = json::value::string(callback_id);

        auto this_hub_connection = shared_from_this();

        // weak_ptr prevents a circular dependency leading to memory leak and other problems
        auto weak_hub_connection = std::weak_ptr<hub_connection_impl>(this_hub_connection);

        m_connection->send(request.serialize())
            .then([set_exception, weak_hub_connection, callback_id](pplx::task<void> send_task)
            {
                try
                {
                    send_task.get();
                }
                catch (const std::exception&)
                {
                    set_exception(std::current_exception());
                    auto hub_connection = weak_hub_connection.lock();
                    if (hub_connection)
                    {
                        hub_connection->m_callback_manager.remove_callback(callback_id);
                    }
                }
            });
    }

    connection_state hub_connection_impl::get_connection_state() const
    {
        return m_connection->get_connection_state();
    }

    // unnamed namespace makes it invisble outside this translation unit
    namespace
    {
        static std::function<void(const json::value&)> create_hub_invocation_callback(const logger& logger,
            std::function<void(const json::value&)> set_result, std::function<void(const std::exception_ptr)> set_exception)
        {
            return[logger, set_result, set_exception](const json::value& message)
            {
                if (message.has_field(_XPLATSTR("R")))
                {
                    set_result(message.at(_XPLATSTR("R")));
                    return;
                }

                if (message.has_field(_XPLATSTR("E")))
                {
                    // TODO: handle hub_exception
                    set_exception(
                        std::make_exception_ptr(
                            std::runtime_error(utility::conversions::to_utf8string(message.at(_XPLATSTR("E")).serialize()))));

                    return;
                }

                if (message.has_field(_XPLATSTR("P")))
                {
                    // TODO: handle progress messages
                    return;
                }

                set_result(json::value::object());
            };
        }
    }
}