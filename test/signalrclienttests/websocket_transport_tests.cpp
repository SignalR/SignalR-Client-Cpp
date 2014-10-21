// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "websocket_transport.h"

using namespace signalr;

class test_websocket_client
{
public:

    pplx::task<void> connect(const web::uri &url)
    {
        return m_connect_function(url);
    }

    pplx::task<void> send(web::experimental::web_sockets::client::websocket_outgoing_message msg)
    {
        return m_send_function(msg);
    }

    pplx::task<web::experimental::web_sockets::client::websocket_incoming_message> receive()
    {
        return m_receive_function();
    }

    pplx::task<void> close()
    {
        return m_close_function();
    }

    void set_connect_function(std::function<pplx::task<void>(const web::uri &url)> connect_function)
    {
        m_connect_function = connect_function;
    }

    void set_send_function(std::function<pplx::task<void>(web::experimental::web_sockets::client::websocket_outgoing_message)> send_function)
    {
        m_send_function = send_function;
    }

    void set_receive_function(std::function<pplx::task<web::experimental::web_sockets::client::websocket_incoming_message>()> receive_function)
    {
        m_receive_function = receive_function;
    }

    void set_close_function(std::function<pplx::task<void>()> close_function)
    {
        m_close_function = close_function;
    }

private:
    std::function<pplx::task<void>(const web::uri &url)> m_connect_function
        = [](const web::uri &){ return pplx::task_from_result(); };

    std::function<pplx::task<void>(web::experimental::web_sockets::client::websocket_outgoing_message)> m_send_function
        = [](web::experimental::web_sockets::client::websocket_outgoing_message msg){ return pplx::task_from_result(); };

    std::function<pplx::task<web::experimental::web_sockets::client::websocket_incoming_message>()> m_receive_function
        = [](){ return pplx::task_from_result(web::experimental::web_sockets::client::websocket_incoming_message()); };

    std::function<pplx::task<void>()> m_close_function
        = [](){ return pplx::task_from_result(); };
};

TEST(websocket_transport_connect, connect_connects_and_starts_receive_loop)
{
    bool connect_called = false, receive_called = false;

    test_websocket_client client;
    client.set_connect_function([&connect_called](const web::uri &) -> pplx::task<void>
    {
        connect_called = true;
        return pplx::task_from_result();
    });

    client.set_receive_function([&receive_called]() -> pplx::task<web::experimental::web_sockets::client::websocket_incoming_message>
    {
        receive_called = true;
        return pplx::task_from_result(web::experimental::web_sockets::client::websocket_incoming_message());
    });

    websocket_transport<test_websocket_client> ws_transport(client);

    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).wait();

    ASSERT_TRUE(connect_called);
    ASSERT_TRUE(receive_called);
}

TEST(websocket_transport_connect, connect_propagates_exceptions)
{
    test_websocket_client client;
    client.set_connect_function([](const web::uri &) -> pplx::task<void>
    {
        throw web::experimental::web_sockets::client::websocket_exception(_XPLATSTR("connecting failed"));
    });

    websocket_transport<test_websocket_client> ws_transport(client);

    try
    {
        ws_transport.connect(_XPLATSTR("http://fakeuri.org")).wait();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::exception &e)
    {
        ASSERT_STREQ("connecting failed", e.what());
    }
}

TEST(websocket_transport_send, send_creates_and_sends_websocket_messages)
{
    bool send_called = false;

    test_websocket_client client;
    client.set_send_function([&send_called](web::experimental::web_sockets::client::websocket_outgoing_message msg) -> pplx::task<void>
    {
        send_called = true;
        return pplx::task_from_result();
    });

    websocket_transport<test_websocket_client> ws_transport(client);

    ws_transport.send(_XPLATSTR("ABC")).wait();

    ASSERT_TRUE(send_called);
}

TEST(websocket_transport_disconnect, disconnect_closes_websocket)
{
    bool close_called = false;

    test_websocket_client client;
    client.set_close_function([&close_called]() -> pplx::task<void>
    {
        close_called = true;
        return pplx::task_from_result();
    });

    websocket_transport<test_websocket_client> ws_transport(client);

    ws_transport.disconnect().wait();

    ASSERT_TRUE(close_called);
}