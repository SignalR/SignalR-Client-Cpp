// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "websocket_transport.h"
#include "logger.h"

namespace signalr
{
    namespace
    {
        void receive_loop(std::shared_ptr<websocket_client> websocket_client, std::shared_ptr<connection_impl> connection, pplx::cancellation_token_source cts);
    }

    websocket_transport::websocket_transport(std::shared_ptr<websocket_client> websocket_client, std::shared_ptr<connection_impl> connection)
        : m_websocket_client(websocket_client), m_connection(connection)
    {
        // we use this cts to check if the receive loop is running so it should be
        // initially cancelled to indicate that the receive loop is not running
        m_receive_loop_cts.cancel();
    }

    websocket_transport::~websocket_transport()
    {
        try
        {
            disconnect().get();
        }
        catch (...) // must not throw from the destructor
        {}
    }

    pplx::task<void> websocket_transport::connect(const web::uri &url)
    {
        if (!m_receive_loop_cts.get_token().is_canceled())
        {
            throw std::runtime_error(utility::conversions::to_utf8string(
                _XPLATSTR("transport already connected")));
        }

        // TODO: prepare request (websocket_client_config)
        pplx::cancellation_token_source receive_loop_cts;
        std::shared_ptr<websocket_client> websocket_client(m_websocket_client);
        pplx::task_completion_event<void> connect_tce;
        auto connection = m_connection;

        m_websocket_client->connect(url)
            .then([websocket_client, connect_tce, receive_loop_cts, connection](pplx::task<void> connect_task)
        {
            try
            {
                connect_task.get();
                receive_loop(websocket_client, connection, receive_loop_cts);
                connect_tce.set();
            }
            catch (const std::exception &e)
            {
                connection->get_logger().log(
                    trace_level::messages,
                    utility::string_t(_XPLATSTR("[websocket transport] exception when connecting to the server: "))
                        .append(utility::conversions::to_string_t(e.what())));

                // TODO: on error(?) - see what we do in the .net client
                receive_loop_cts.cancel();
                connect_tce.set_exception(std::current_exception());
            }
        });

        m_receive_loop_cts = receive_loop_cts;

        return pplx::create_task(connect_tce);
    }

    pplx::task<void> websocket_transport::send(const utility::string_t &data)
    {
        // send will throw if client not connected
        return m_websocket_client->send(data);
    }

    pplx::task<void> websocket_transport::disconnect()
    {
        m_receive_loop_cts.cancel();

        auto logger = m_connection->get_logger();

        return m_websocket_client->close()
            .then([logger](pplx::task<void> close_task)
            mutable {
                try
                {
                    close_task.get();
                }
                catch (const std::exception &e)
                {
                    logger.log(
                        trace_level::messages,
                        utility::string_t(_XPLATSTR("[websocket transport] exception when closing websocket: "))
                        .append(utility::conversions::to_string_t(e.what())));
                }
            });
    }

    // unnamed namespace makes this function invisible/unusable in other translation units
    namespace
    {
        void receive_loop(std::shared_ptr<websocket_client> websocket_client, std::shared_ptr<connection_impl> connection, pplx::cancellation_token_source cts)
        {
            websocket_client->receive()
                // There are two cases when we exit the loop. The first case is implicit - we pass the cancellation_token
                // to `then` (note this is after the lambda body) and if the token is cancelled the continuation will not
                // run at all. The second - explicit - case happens if the token gets cancelled after the continuation has
                // been started in which case we just stop the loop by not scheduling another receive task.
                .then([websocket_client, connection, cts](pplx::task<std::string> receive_task)
            {
                try
                {
                    auto msg_body = receive_task.get();

                    // TODO: process message

                    if (!pplx::is_task_cancellation_requested())
                    {
                        receive_loop(websocket_client, connection, cts);
                    }

                    return;
                }
                // TODO: report error, close websocket (when appropriate)
                catch (const web_sockets::client::websocket_exception& e)
                {
                    connection->get_logger().log(
                        trace_level::messages,
                        utility::string_t(_XPLATSTR("[websocket transport] websocket exception when receiving data: "))
                        .append(utility::conversions::to_string_t(e.what())));
                }
                catch (const pplx::task_canceled& e)
                {
                    connection->get_logger().log(
                        trace_level::messages,
                        utility::string_t(_XPLATSTR("[websocket transport] receive task cancelled: "))
                        .append(utility::conversions::to_string_t(e.what())));
                }
                catch (const std::exception& e)
                {
                    connection->get_logger().log(
                        trace_level::messages,
                        utility::string_t(_XPLATSTR("[websocket transport] error receiving response from websocket: "))
                        .append(utility::conversions::to_string_t(e.what())));
                }
                catch (...)
                {
                    connection->get_logger().log(
                        trace_level::messages,
                        utility::string_t(_XPLATSTR("[websocket transport] unknown error occurred when receiving response from websocket")));
                }

                cts.cancel();
            }, cts.get_token());
        }
    }
}