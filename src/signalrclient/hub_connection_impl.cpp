// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "hub_connection_impl.h"

namespace signalr
{
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
        m_logger(log_writer, trace_level)
    { }

    void hub_connection_impl::initialize()
    {
        auto this_hub_connection = shared_from_this();

        // weak_ptr prevents a circular dependency leading to memory leak and other problems
        auto weak_hub_connection = std::weak_ptr<hub_connection_impl>(this_hub_connection);

        m_connection->set_message_received([weak_hub_connection](const utility::string_t& message)
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
        return m_connection->stop();
    }

    // TODO: this method should take json::value to avoid duplicate parsing
    void hub_connection_impl::process_message(const utility::string_t& message)
    {
        //TODO: handle non-hub messages
        auto message_json = json::value::parse(message);

        if (!message_json[_XPLATSTR("P")].is_null())
        {
            // TODO: handle progress messages
            return;
        }

        if (!message_json[_XPLATSTR("I")].is_null())
        {
            // TODO: handle invocation result
            return;
        }

        auto hub_name = message_json[_XPLATSTR("H")].as_string();
        auto iter = m_proxies.find(hub_name);
        if (iter != m_proxies.end())
        {
            iter->second->invoke_event(message_json[_XPLATSTR("M")].as_string(), message_json[_XPLATSTR("A")]);
        }
        else
        {
            m_logger.log(trace_level::info,
                utility::string_t(_XPLATSTR("no proxy found for hub invocation. hub: "))
                    .append(hub_name).append(_XPLATSTR(", method: ")).append(message_json[_XPLATSTR("M")].as_string()));
        }
    }

    connection_state hub_connection_impl::get_connection_state() const
    {
        return m_connection->get_connection_state();
    }
}