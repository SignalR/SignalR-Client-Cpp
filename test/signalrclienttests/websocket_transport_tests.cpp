// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "test_websocket_client.h"
#include "websocket_transport.h"

using namespace signalr;
using namespace web::experimental;

TEST(websocket_transport_connect, connect_connects_and_starts_receive_loop)
{
    bool connect_called = false, receive_called = false;

    auto client = std::make_shared<test_websocket_client>();

    client->set_connect_function([&connect_called](const web::uri &) -> pplx::task<void>
    {
        connect_called = true;
        return pplx::task_from_result();
    });

    client->set_receive_function([&receive_called]()->pplx::task<std::string>
    {
        receive_called = true;
        return pplx::task_from_result(std::string(""));
    });

    websocket_transport ws_transport(client);

    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).get();

    ASSERT_TRUE(connect_called);
    ASSERT_TRUE(receive_called);
}

TEST(websocket_transport_connect, connect_propagates_exceptions)
{
    auto client = std::make_shared<test_websocket_client>();
    client->set_connect_function([](const web::uri &) -> pplx::task<void>
    {
        throw web_sockets::client::websocket_exception(_XPLATSTR("connecting failed"));
    });

    websocket_transport ws_transport(client);

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

TEST(websocket_transport_connect, cannot_call_connect_on_already_connected_transport)
{
    auto client = std::make_shared<test_websocket_client>();
    websocket_transport ws_transport(client);

    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).wait();

    try
    {
        ws_transport.connect(_XPLATSTR("http://fakeuri.org")).wait();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::exception &e)
    {
        ASSERT_STREQ("transport already connected", e.what());
    }
}

TEST(websocket_transport_connect, can_connect_after_disconnecting)
{

    auto client = std::make_shared<test_websocket_client>();
    websocket_transport ws_transport(client);

    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).get();
    ws_transport.disconnect().get();
    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).get();
    // shouldn't throw or crash 
}

TEST(websocket_transport_send, send_creates_and_sends_websocket_messages)
{
    bool send_called = false;

    auto client = std::make_shared<test_websocket_client>();

    client->set_send_function([&send_called](const utility::string_t&) -> pplx::task<void>
    {
        send_called = true;
        return pplx::task_from_result();
    });

    websocket_transport ws_transport(client);

    ws_transport.send(_XPLATSTR("ABC")).wait();

    ASSERT_TRUE(send_called);
}

TEST(websocket_transport_disconnect, disconnect_closes_websocket)
{
    bool close_called = false;

    auto client = std::make_shared<test_websocket_client>();

    client->set_close_function([&close_called]() -> pplx::task<void>
    {
        close_called = true;
        return pplx::task_from_result();
    });

    websocket_transport ws_transport(client);

    ws_transport.disconnect().get();

    ASSERT_TRUE(close_called);
}

TEST(websocket_transport_disconnect, disconnect_does_not_throw)
{
    auto client = std::make_shared<test_websocket_client>();

    client->set_close_function([]() -> pplx::task<void>
    {
        return pplx::task_from_exception<void>(std::exception());
    });

    websocket_transport ws_transport(client);
    ws_transport.disconnect().get();
}

TEST(websocket_transport_disconnect, receive_not_called_after_disconnect)
{
    auto client = std::make_shared<test_websocket_client>();

    pplx::task_completion_event<std::string> receive_task_tce;

    client->set_close_function([&receive_task_tce]()
    {
        // unblock receive
        receive_task_tce.set(std::string(""));
        return pplx::task_from_result();
    });

    int num_called = 0;

    client->set_receive_function([&receive_task_tce, &num_called]() -> pplx::task<std::string>
    {
        num_called++;
        return pplx::create_task(receive_task_tce);
    });

    websocket_transport ws_transport(client);

    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).get();
    ws_transport.disconnect().get();

    receive_task_tce = pplx::task_completion_event<std::string>();
    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).get();
    ws_transport.disconnect().get();

    ASSERT_EQ(2, num_called);
}