// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include <cpprest\asyncrt_utils.h>
#include "connection_impl.h"
#include "request_sender.h"
#include "url_builder.h"

namespace signalr
{
    std::shared_ptr<connection_impl> connection_impl::create(const utility::string_t& url, const utility::string_t& query_string,
        trace_level trace_level, std::shared_ptr<log_writer> log_writer)
    {
        return connection_impl::create(url, query_string, trace_level, log_writer, std::make_unique<web_request_factory>(), std::make_unique<transport_factory>());
    }

    std::shared_ptr<connection_impl> connection_impl::create(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level,
        std::shared_ptr<log_writer> log_writer, std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory)
    {
        return std::shared_ptr<connection_impl>(new connection_impl(url, query_string, trace_level, log_writer, std::move(web_request_factory), std::move(transport_factory)));
    }

    connection_impl::connection_impl(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level, std::shared_ptr<log_writer> log_writer,
        std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory)
        : m_base_url(url), m_query_string(query_string), m_connection_state(connection_state::disconnected),
        m_logger(log_writer, trace_level), m_transport(nullptr), m_web_request_factory(std::move(web_request_factory)),
        m_transport_factory(std::move(transport_factory)), m_message_received([](const utility::string_t&){})
    { }

    pplx::task<void> connection_impl::start()
    {
        if (!change_state(connection_state::disconnected, connection_state::connecting))
        {
            throw std::runtime_error("cannot start a connection that is not in the disconnected state");
        }

        // there should not be any active transport at this point
        _ASSERTE(!m_transport);

        pplx::task_completion_event<void> start_tce;
        auto connection = shared_from_this();

        request_sender::negotiate(*m_web_request_factory, m_base_url, m_query_string)
            .then([connection](negotiation_response negotiation_response)
            {
                if (!negotiation_response.try_websockets)
                {
                    return pplx::task_from_exception<void>(
                        std::runtime_error("websockets not supported on the server and there is no fallback transport"));
                }

                connection->m_connection_token = negotiation_response.connection_token;

                auto weak_connection = std::weak_ptr<connection_impl>(connection);
                auto process_response_callback = [weak_connection](utility::string_t response)
                {
                    auto connection = weak_connection.lock();
                    if (connection)
                    {
                        connection->process_response(response);
                    }
                };

                connection->m_transport = connection->m_transport_factory->create_transport(
                    transport_type::websockets, connection->m_logger, process_response_callback);

                return connection->send_connect_request(negotiation_response.connection_token);
            })
            .then([connection]()
            {
                return request_sender::start(*connection->m_web_request_factory, connection->m_base_url,
                    connection->m_transport->get_transport_type(), connection->m_connection_token, connection->m_query_string);
            })
            .then([start_tce, connection](pplx::task<void> previous_task)
            {
                try
                {
                    previous_task.get();
                    connection->change_state(connection_state::connecting, connection_state::connected);
                    start_tce.set();
                }
                catch (const std::exception &e)
                {
                    connection->m_logger.log(
                        trace_level::errors,
                        utility::string_t(_XPLATSTR("connection could not be started due to: "))
                            .append(utility::conversions::to_string_t(e.what())));

                    connection->m_transport.reset();
                    connection->change_state(connection_state::connecting, connection_state::disconnected);
                    start_tce.set_exception(std::current_exception());
                }
            });

        return pplx::create_task(start_tce);
    }

    pplx::task<void> connection_impl::send_connect_request(const utility::string_t& connection_token)
    {
        auto connection = shared_from_this();

        auto connect_url = url_builder::build_connect(m_base_url, connection->m_transport->get_transport_type(),
            connection_token, m_query_string);

        pplx::task_completion_event<void> connect_request_tce;
        m_connect_request_tce = connect_request_tce;

        m_transport->connect(connect_url)
            .then([connection](pplx::task<void> connect_task)
            {
                try
                {
                    connect_task.get();
                }
                catch (const std::exception& e)
                {
                    connection->m_logger.log(
                        trace_level::errors,
                        utility::string_t(_XPLATSTR("transport could not connect due to: "))
                            .append(utility::conversions::to_string_t(e.what())));

                    connection->m_connect_request_tce.set_exception(std::current_exception());
                }
            });

        return pplx::create_task(m_connect_request_tce);
    }

    void connection_impl::process_response(const utility::string_t& response)
    {
        m_logger.log(trace_level::messages,
            utility::string_t(_XPLATSTR("processing message: ")).append(response));

        try
        {
            auto result = web::json::value::parse(response);

            auto messages = result[_XPLATSTR("M")];
            if (!messages.is_null() && messages.is_array())
            {
                if (result[_XPLATSTR("S")].is_integer() && result[_XPLATSTR("S")].as_integer() == 1)
                {
                    m_connect_request_tce.set();
                }

                for (auto& m : messages.as_array())
                {
                    try
                    {
                        m_message_received(m.serialize());
                    }
                    catch (const std::exception &e)
                    {
                        m_logger.log(
                            trace_level::errors,
                            utility::string_t(_XPLATSTR("message_received callback threw an exception: "))
                                .append(utility::conversions::to_string_t(e.what())));

                        // TODO: call on error callback
                    }
                    catch (...)
                    {
                        m_logger.log(
                            trace_level::errors,
                            utility::string_t(_XPLATSTR("message_received callback threw an unknown exception.")));

                        // TODO: call on error callback
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            m_logger.log(trace_level::errors, utility::string_t(_XPLATSTR("error occured when parsing response: "))
                .append(utility::conversions::to_string_t(e.what()))
                .append(_XPLATSTR(". response: "))
                .append(response));
        }
    }

    pplx::task<void> connection_impl::send(utility::string_t data)
    {
        auto connection_state = get_connection_state();
        if (connection_state != connection_state::connected)
        {
            return pplx::task_from_exception<void>(std::runtime_error(
                std::string{ "cannot send data when the connection is not in the connected state. current connection state: " }
                    .append(utility::conversions::to_utf8string(translate_connection_state(connection_state)))));
        }

        auto logger = m_logger;

        return m_transport->send(data)
            .then([logger](pplx::task<void> send_task)
            mutable {
                try
                {
                    send_task.get();
                }
                catch (const std::exception &e)
                {
                    // TODO: call on error callback?
                    logger.log(
                        trace_level::errors,
                        utility::string_t(_XPLATSTR("error sending data: "))
                        .append(utility::conversions::to_string_t(e.what())));

                    throw;
                }
            });
    }

    connection_state connection_impl::get_connection_state() const
    {
        return m_connection_state.load();
    }

    void connection_impl::set_message_received(const std::function<void(const utility::string_t&)>& message_received)
    {
        auto connection_state = get_connection_state();
        if (connection_state != connection_state::disconnected)
        {
            throw std::runtime_error(
                std::string{ "cannot set the callback when the connection is not in the disconnected state. current connection state: " }
            .append(utility::conversions::to_utf8string(translate_connection_state(connection_state))));
        }

        m_message_received = message_received;
    }

    bool connection_impl::change_state(connection_state old_state, connection_state new_state)
    {
        connection_state expected_state{ old_state };

        if (m_connection_state.compare_exchange_strong(expected_state, new_state, std::memory_order_seq_cst))
        {
            m_logger.log(
                trace_level::state_changes,
                translate_connection_state(old_state)
                .append(_XPLATSTR(" -> "))
                .append(translate_connection_state(new_state)));

            // TODO: invoke state_changed callback

            return true;
        }

        return false;
    }

    utility::string_t connection_impl::translate_connection_state(connection_state state)
    {
        switch (state)
        {
        case connection_state::connecting:
            return _XPLATSTR("connecting");
        case connection_state::connected:
            return _XPLATSTR("connected");
        case connection_state::reconnecting:
            return _XPLATSTR("reconnecting");
        case connection_state::disconnected:
            return _XPLATSTR("disconnected");
        default:
            _ASSERTE(false);
            return _XPLATSTR("(unknown)");
        }
    }
}