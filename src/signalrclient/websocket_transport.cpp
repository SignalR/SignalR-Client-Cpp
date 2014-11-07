// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "websocket_transport.h"

std::string debug_log;

namespace signalr
{
    namespace
    {
        void receive_loop(std::shared_ptr<websocket_client> websocket_client, pplx::cancellation_token_source cts);
    }

    websocket_transport::websocket_transport(std::shared_ptr<websocket_client> websocket_client)
        : m_websocket_client(websocket_client)
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

        m_websocket_client->connect(url)
            .then([websocket_client, connect_tce, receive_loop_cts](pplx::task<void> connect_task)
        {
            try
            {
                connect_task.get();
                receive_loop(websocket_client, receive_loop_cts);
                connect_tce.set();
            }
            catch (const std::exception &e)
            {
                // TODO: logging, on error(?) - see what we do in the .net client
                receive_loop_cts.cancel();
                connect_tce.set_exception(e);
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

        return m_websocket_client->close()
            .then([](pplx::task<void> close_task)
            {
                try
                {
                    close_task.get();
                }
                catch (const std::exception &)
                {
                    // TODO:log
                }
            });
    }

    // unnamed namespace makes this function invisible for other translation units 
    namespace
    {
        void receive_loop(std::shared_ptr<websocket_client> websocket_client, pplx::cancellation_token_source cts)
        {
            websocket_client->receive()
                .then([websocket_client, cts](pplx::task<std::string> receive_task)
            {
                try
                {
                    auto msg_body = receive_task.get();

                    // TODO: process message

                    if (!pplx::is_task_cancellation_requested())
                    {
                        receive_loop(websocket_client, cts);
                    }
                }

                // TODO: log, report error, close websocket (when appropriate)
                catch (const web_sockets::client::websocket_exception&)
                {
                }
                catch (const pplx::task_canceled &)
                {
                }
                catch (const std::exception&)
                {
                }
                catch (...)
                {
                }

                cts.cancel();

            }, cts.get_token());
        }
    }
}