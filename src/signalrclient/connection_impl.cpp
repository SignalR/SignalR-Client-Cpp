// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include <thread>
#include "connection_impl.h"
#include "request_sender.h"
#include "url_builder.h"
#include "trace_log_writer.h"

namespace signalr
{
    std::shared_ptr<connection_impl> connection_impl::create(const utility::string_t& url, const utility::string_t& query_string,
        trace_level trace_level, const std::shared_ptr<log_writer>& log_writer)
    {
        return connection_impl::create(url, query_string, trace_level, log_writer, std::make_unique<web_request_factory>(), std::make_unique<transport_factory>());
    }

    std::shared_ptr<connection_impl> connection_impl::create(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level,
        const std::shared_ptr<log_writer>& log_writer, std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory)
    {
        return std::shared_ptr<connection_impl>(new connection_impl(url, query_string, trace_level,
            log_writer ? log_writer : std::make_shared<trace_log_writer>(), std::move(web_request_factory), std::move(transport_factory)));
    }

    connection_impl::connection_impl(const utility::string_t& url, const utility::string_t& query_string, trace_level trace_level, const std::shared_ptr<log_writer>& log_writer,
        std::unique_ptr<web_request_factory> web_request_factory, std::unique_ptr<transport_factory> transport_factory)
        : m_base_url(url), m_query_string(query_string), m_connection_state(connection_state::disconnected),
        m_logger(log_writer, trace_level), m_transport(nullptr), m_web_request_factory(std::move(web_request_factory)),
        m_transport_factory(std::move(transport_factory)), m_message_received([](const web::json::value&){})
    { }

    connection_impl::~connection_impl()
    {
        try
        {
            shutdown().get();
        }
        catch (const pplx::task_canceled&)
        {
            // because we are in the dtor and the `connection_imp` is ref counted we should not get the `task_canceled`
            // exception because it would indicate that some other thread/task still holds reference to this instance
            // so how come we are in the dtor?
            _ASSERTE(false);
            return;
        }
        catch (...) // must not throw from destructors
        { }

        m_transport = nullptr;
        change_state(connection_state::disconnected);
    }

    pplx::task<void> connection_impl::start()
    {
        {
            std::lock_guard<std::mutex> lock(m_stop_lock);
            if (!change_state(connection_state::disconnected, connection_state::connecting))
            {
                return pplx::task_from_exception<void>(
                    std::runtime_error("cannot start a connection that is not in the disconnected state"));
            }

            // there should not be any active transport at this point
            _ASSERTE(!m_transport);

            m_disconnect_cts = pplx::cancellation_token_source();
            m_start_completed_event.reset();
            m_message_id = m_groups_token = _XPLATSTR("");
        }

        pplx::task_completion_event<void> start_tce;

        auto connection = shared_from_this();

        pplx::task_from_result()
            .then([connection]()
            {
                return request_sender::negotiate(*connection->m_web_request_factory, connection->m_base_url,
                    connection->m_connection_data, connection->m_query_string, connection->m_headers);
            }, m_disconnect_cts.get_token())
            .then([connection](negotiation_response negotiation_response)
            {
                connection->m_last_message_at = utility::datetime::utc_now();

                return connection->start_transport(negotiation_response)
                    .then([connection, negotiation_response](std::shared_ptr<transport> transport)
                    {
                        connection->m_transport = transport;
                        connection->m_connection_token = negotiation_response.connection_token;
                    });
            }, m_disconnect_cts.get_token())
            .then([connection]()
            {
                return request_sender::start(*connection->m_web_request_factory, connection->m_base_url,
                    connection->m_transport->get_transport_type(), connection->m_connection_token,
                    connection->m_connection_data, connection->m_query_string, connection->m_headers);
            }, m_disconnect_cts.get_token())
            .then([start_tce, connection](pplx::task<void> previous_task)
            {
                try
                {
                    previous_task.get();
                    if (!connection->change_state(connection_state::connecting, connection_state::connected))
                    {
                        connection->m_logger.log(trace_level::errors,
                            utility::string_t(_XPLATSTR("internal error - transition from an unexpected state. expected state: connecting, actual state: "))
                            .append(translate_connection_state(connection->get_connection_state())));

                        _ASSERTE(false);
                    }

                    connection->m_start_completed_event.set();
                    start_tce.set();
                }
                catch (const std::exception &e)
                {
                    auto task_canceled_exception = dynamic_cast<const pplx::task_canceled *>(&e);
                    if (task_canceled_exception)
                    {
                        connection->m_logger.log(trace_level::info,
                            _XPLATSTR("starting the connection has been cancelled."));
                    }
                    else
                    {
                        connection->m_logger.log(trace_level::errors,
                            utility::string_t(_XPLATSTR("connection could not be started due to: "))
                            .append(utility::conversions::to_string_t(e.what())));
                    }

                    connection->m_transport = nullptr;
                    connection->change_state(connection_state::disconnected);
                    connection->m_start_completed_event.set();
                    start_tce.set_exception(std::current_exception());
                }
            });

        return pplx::create_task(start_tce);
    }

    pplx::task<std::shared_ptr<transport>> connection_impl::start_transport(negotiation_response negotiation_response)
    {
        if (!negotiation_response.try_websockets)
        {
            return pplx::task_from_exception<std::shared_ptr<transport>>(
            std::runtime_error("websockets not supported on the server and there is no fallback transport"));
        }

        auto connection = shared_from_this();

        pplx::task_completion_event<void> connect_request_tce;

        auto weak_connection = std::weak_ptr<connection_impl>(connection);
        auto process_response_callback = [weak_connection, connect_request_tce](const utility::string_t& response)
        {
            auto connection = weak_connection.lock();
            if (connection)
            {
                connection->process_response(response, connect_request_tce);
            }
        };

        auto error_callback = [connect_request_tce](const std::exception &e)
        {
            connect_request_tce.set_exception(e);
        };

        auto transport = connection->m_transport_factory->create_transport(
            transport_type::websockets, connection->m_logger, connection->m_headers, process_response_callback, error_callback);

        pplx::create_task([negotiation_response, connect_request_tce]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(negotiation_response.transport_connect_timeout));
            connect_request_tce.set_exception(std::runtime_error("transport timed out when trying to connect"));
        });

        return connection->send_connect_request(transport, negotiation_response.connection_token, connect_request_tce)
            .then([transport](){ return pplx::task_from_result(transport); });
    }

    pplx::task<void> connection_impl::send_connect_request(const std::shared_ptr<transport>& transport,
        const utility::string_t& connection_token, const pplx::task_completion_event<void>& connect_request_tce)
    {
        auto logger = m_logger;
        auto connect_url = url_builder::build_connect(m_base_url, transport->get_transport_type(),
            connection_token, m_connection_data, m_query_string);

        transport->connect(connect_url)
            .then([connect_request_tce, logger](pplx::task<void> connect_task)
            mutable {
                try
                {
                    connect_task.get();
                }
                catch (const std::exception& e)
                {
                    logger.log(
                        trace_level::errors,
                        utility::string_t(_XPLATSTR("transport could not connect due to: "))
                            .append(utility::conversions::to_string_t(e.what())));

                    connect_request_tce.set_exception(std::current_exception());
                }
            });

        return pplx::create_task(connect_request_tce);
    }

    void connection_impl::process_response(const utility::string_t& response, const pplx::task_completion_event<void>& connect_request_tce)
    {
        m_logger.log(trace_level::messages,
            utility::string_t(_XPLATSTR("processing message: ")).append(response));

        try
        {
            m_last_message_at = utility::datetime::utc_now();

            const auto result = web::json::value::parse(response);

            if (!result.is_object())
            {
                m_logger.log(trace_level::info, utility::string_t(_XPLATSTR("unexpected response received from the server: "))
                    .append(response));

                return;
            }

            if (result.has_field(_XPLATSTR("I")))
            {
                invoke_message_received(result);
                return;
            }


            // The assumption is that we cannot start reconnecting when a message is being processed otherwise there
            // a data race occurs - we could read `m_groups_token` and `m_message_id` while they are being set below.
            // This can't happen right now since in the `websocket_transport.receive_loop` we either process the message
            // or effectively invoke reconnect.
            if (result.has_field(_XPLATSTR("G")) && result.at(_XPLATSTR("G")).is_string())
            {
                m_groups_token = result.at(_XPLATSTR("G")).as_string();
            }

            if (result.has_field(_XPLATSTR("M")) && result.at(_XPLATSTR("M")).is_array())
            {
                _ASSERTE(result.has_field(_XPLATSTR("C")));
                _ASSERTE(result.at(_XPLATSTR("C")).is_string());

                m_message_id = result.at(_XPLATSTR("C")).as_string();

                if (result.has_field(_XPLATSTR("S")) && result.at(_XPLATSTR("S")).is_integer() && result.at(_XPLATSTR("S")).as_integer() == 1)
                {
                    connect_request_tce.set();
                }

                for (auto& m : result.at(_XPLATSTR("M")).as_array())
                {
                    invoke_message_received(m);
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

    void connection_impl::invoke_message_received(const web::json::value& message)
    {
        try
        {
            m_message_received(message);
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

    pplx::task<void> connection_impl::send(const utility::string_t& data)
    {
        // To prevent an (unlikely) condition where the transport is nulled out after we checked the connection_state
        // and before sending data we store the pointer in the local variable. In this case `send()` will throw but
        // we won't crash.
        auto transport = m_transport;

        auto connection_state = get_connection_state();
        if (connection_state != connection_state::connected || !transport)
        {
            return pplx::task_from_exception<void>(std::runtime_error(
                std::string{ "cannot send data when the connection is not in the connected state. current connection state: " }
                    .append(utility::conversions::to_utf8string(translate_connection_state(connection_state)))));
        }

        auto logger = m_logger;

        return transport->send(data)
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

    pplx::task<void> connection_impl::stop()
    {
        auto connection = shared_from_this();
        return shutdown()
            .then([connection]()
            {
                // the lock prevents a race where the user calls `stop` on a disconnected connection and calls `start`
                // on a different thread at the same time. In this case we must not null out the transport if we are
                // not in the `disconnecting` state to not affect the 'start' invocation.
                std::lock_guard<std::mutex> lock(connection->m_stop_lock);
                if (connection->change_state(connection_state::disconnecting, connection_state::disconnected))
                {
                    // we do let the exception through (especially the task_canceled exception)
                    connection->m_transport = nullptr;
                }
            });
    }

    // This function is called from the dtor so you must not use `shared_from_this` here (it will throw).
    pplx::task<void> connection_impl::shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(m_stop_lock);
            auto current_state = get_connection_state();
            if (current_state == connection_state::disconnected)
            {
                return pplx::task_from_result();
            }

            if (current_state == connection_state::disconnecting)
            {
                // cancelled task will be returned if `stop` was called while another `stop` was already in progress.
                // This is to prevent from resetting the `m_transport` in the upstream callers because doing so might
                // affect the other invocation which is using it.
                auto cts = pplx::cancellation_token_source();
                cts.cancel();
                return pplx::create_task([](){}, cts.get_token());
            }

            // we request a cancellation of the ongoing start request (if any) and wait until it is cancelled
            m_disconnect_cts.cancel();

            while (m_start_completed_event.wait(60000) != 0)
            {
                m_logger.log(trace_level::errors,
                    utility::string_t(_XPLATSTR("internal error - stopping the connection is still waiting for the start operation to finish which should have already finished or timed out")));
            }

            // at this point we are either in the connected or disconnected state. If we are in the disconnected state
            // we must break because the transport have already been nulled out.
            if (!change_state(connection_state::connected, connection_state::disconnecting))
            {
                return pplx::task_from_result();
            }
        }

        // This is fire and forget because we don't really care about the result
        request_sender::abort(*m_web_request_factory, m_base_url, m_transport->get_transport_type(), m_connection_token, m_connection_data, m_query_string, m_headers)
            .then([](pplx::task<utility::string_t> abort_task)
            {
                try
                {
                    abort_task.get();
                }
                catch (...)
                {
                    // We don't care about the result and even if the request failed there is not much we can do. We do
                    // need to observe the exception though to prevent from crash due to unobserved exception exception.
                }
            });

        return m_transport->disconnect();
    }

    connection_state connection_impl::get_connection_state() const
    {
        return m_connection_state.load();
    }

    void connection_impl::set_message_received_string(const std::function<void(const utility::string_t&)>& message_received)
    {
        set_message_received_json([message_received](const web::json::value& payload)
        {
            message_received(payload.serialize());
        });
    }

    void connection_impl::set_message_received_json(const std::function<void(const web::json::value&)>& message_received)
    {
        ensure_disconnected("cannot set the callback when the connection is not in the disconnected state. ");
        m_message_received = message_received;
    }

    void connection_impl::set_connection_data(const utility::string_t& connection_data)
    {
        _ASSERTE(get_connection_state() == connection_state::disconnected);

        m_connection_data = connection_data;
    }

    void connection_impl::set_headers(const std::unordered_map<utility::string_t, utility::string_t>& headers)
    {
        ensure_disconnected("cannot set headers when the connection is not in the disconnected state. ");
        m_headers = headers;
    }

    void connection_impl::ensure_disconnected(const std::string& error_message)
    {
        auto connection_state = get_connection_state();
        if (connection_state != connection_state::disconnected)
        {
            throw std::runtime_error(error_message + std::string{"current connection state: "}
                .append(utility::conversions::to_utf8string(translate_connection_state(connection_state))));
        }
    }

    bool connection_impl::change_state(connection_state old_state, connection_state new_state)
    {
        if (m_connection_state.compare_exchange_strong(old_state, new_state, std::memory_order_seq_cst))
        {
            handle_connection_state_change(old_state, new_state);
            return true;
        }

        return false;
    }

    connection_state connection_impl::change_state(connection_state new_state)
    {
        auto old_state = m_connection_state.exchange(new_state);
        if (old_state != new_state)
        {
            handle_connection_state_change(old_state, new_state);
        }

        return old_state;
    }

    void connection_impl::handle_connection_state_change(connection_state old_state, connection_state new_state)
    {
        m_logger.log(
            trace_level::state_changes,
            translate_connection_state(old_state)
            .append(_XPLATSTR(" -> "))
            .append(translate_connection_state(new_state)));

        // TODO: invoke state_changed callback
        // Words of wisdom:
        // "Be extra careful when you add this callback, because this is sometimes being called with the m_stop_lock.
        // This could lead to interesting problems.For example, you could run into a segfault if the connection is
        // stopped while / after transitioning into the connecting state."
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
        case connection_state::disconnecting:
            return _XPLATSTR("disconnecting");
        case connection_state::disconnected:
            return _XPLATSTR("disconnected");
        default:
            _ASSERTE(false);
            return _XPLATSTR("(unknown)");
        }
    }
}