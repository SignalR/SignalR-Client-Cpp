// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "websocket_transport.h"

namespace signalr
{
    websocket_transport::websocket_transport(std::unique_ptr<websocket_client> websocket_client)
        : m_websocket_client(std::move(websocket_client))
    {}

    pplx::task<void> websocket_transport::connect(const web::uri &url) 
    {
        // TODO: prepare request (websocket_client_config)

        m_websocket_client->connect(url)
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

    pplx::task<void> websocket_transport::send(const utility::string_t &data)
    {
        // send will throw if client not connected
        return m_websocket_client->send(data);
    }

    pplx::task<void> websocket_transport::disconnect()
    {
        return m_websocket_client->close();
    }


    void websocket_transport::receive_loop()
    {
        // TODO: there is a race where m_websocket_client is destroyed in the destructor
        // but the receive loop is not yet stopped since it is running on a different thread
        // in which case we crash when try calling m_websocket_client->receive()
        m_websocket_client->receive().then([this](pplx::task<std::string> receive_task)
        {
            try
            {
                auto msg_body = receive_task.get();
                receive_loop();
            }
            catch (web_sockets::client::websocket_exception &)
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
}