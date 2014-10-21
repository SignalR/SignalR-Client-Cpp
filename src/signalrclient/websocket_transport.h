// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include <cpprest/ws_client.h>
#include "url_builder.h"

namespace signalr
{
    template<typename T = web::experimental::web_sockets::client::websocket_client> // testing
    class websocket_transport
    {
        //TODO: make non-copyable or implement move =operator (needed at all?)

    public:
        websocket_transport() : websocket_transport(T())
        { }

        websocket_transport(T websocket_client) : m_websocket_client(websocket_client)
        { }

        pplx::task<void> connect(const web::uri &url)
        {
            // TODO: prepare request (websocket_client_config)

            m_websocket_client.connect(url)
                .then([this](pplx::task<void> connect_task){
                try
                {
                    connect_task.wait();
                    receive_loop();
                    this->m_connect_tce.set();
                }
                catch (const std::exception &e)
                {
                    this->m_connect_tce.set_exception(e);
                }
            });

            return pplx::create_task(m_connect_tce);
        }

        pplx::task<void> send(const utility::string_t &data)
        {
            web::experimental::web_sockets::client::websocket_outgoing_message msg;
            msg.set_utf8_message(utility::conversions::to_utf8string(data));

            // send will throw if client not connected
            return m_websocket_client.send(msg);
        }

        pplx::task<void> disconnect()
        {
            return m_websocket_client.close();
        }

    private:
        T m_websocket_client;

        pplx::task_completion_event<void> m_connect_tce;

        void receive_loop()
        {
            m_websocket_client.receive().then([this](pplx::task<web::experimental::web_sockets::client::websocket_incoming_message> msg_task)
            {
                try
                {
                    auto msg = msg_task.get();
                    msg.extract_string().then([this](std::string msg_body)
                    {
                        receive_loop();
                    });
                }
                catch (web::experimental::web_sockets::client::websocket_exception &)
                {
                    // TODO: should close the websocket?
                    // TODO: report error
                }
                catch (std::exception &)
                {
                    // TODO: should close the websocket?
                    // TODO: report error
                }
            });
        }
    };
}