// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "stdafx.h"
#include <string>
#include "cpprest\details\basic_types.h"
#include "cpprest\json.h"
#include "connection.h"
#include "hub_connection.h"

extern utility::string_t url;

TEST(hub_connection_tests, connection_status_start_stop_start_reconnect)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url + U("SignalR"));
    auto weak_hub_conn = std::weak_ptr<signalr::hub_connection>(hub_conn);
    auto reconnecting_event = std::make_shared<pplx::event>();
    auto reconnected_event = std::make_shared<pplx::event>();

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_conn->set_reconnecting([weak_hub_conn, reconnecting_event]()
    {
        auto conn = weak_hub_conn.lock();
        if (conn)
        {
            ASSERT_EQ(conn->get_connection_state(), signalr::connection_state::reconnecting);
        }
        reconnecting_event->set();
    });

    hub_conn->set_reconnected([weak_hub_conn, reconnected_event]()
    {
        auto conn = weak_hub_conn.lock();
        if (conn)
        {
            ASSERT_EQ(conn->get_connection_state(), signalr::connection_state::connected);
        }
        reconnected_event->set();
    });

    hub_conn->start().get();
    ASSERT_EQ(hub_conn->get_connection_state(), signalr::connection_state::connected);

    hub_conn->stop().get();
    ASSERT_EQ(hub_conn->get_connection_state(), signalr::connection_state::disconnected);

    hub_conn->start().get();
    ASSERT_EQ(hub_conn->get_connection_state(), signalr::connection_state::connected);

    try
    {
        hub_proxy.invoke<web::json::value>(U("forceReconnect")).get();
    }
    catch (...)
    {
    }

    ASSERT_FALSE(reconnecting_event->wait(2000));
    ASSERT_FALSE(reconnected_event->wait(2000));
}

TEST(hub_connection_tests, send_message)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url + U("SignalR"));
    auto message = std::make_shared<utility::string_t>();
    auto received_event = std::make_shared<pplx::event>();

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_proxy.on(U("displayMessage"), [message, received_event](const web::json::value& arguments)
    {
        *message = arguments.serialize();
        received_event->set();
    });

    hub_conn->start().then([&hub_proxy]()
    {
        web::json::value obj{};
        obj[0] = web::json::value(U("test"));

        return hub_proxy.invoke<web::json::value>(U("displayMessage"), obj);

    }).get();

    ASSERT_FALSE(received_event->wait(200));

    ASSERT_EQ(*message, U("[\"Send: test\"]"));
}

TEST(hub_connection_tests, send_message_return)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url + U("SignalR"));

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    auto test = hub_conn->start().then([&hub_proxy]()
    {
        web::json::value obj{};
        obj[0] = web::json::value(U("test"));

        return hub_proxy.invoke<web::json::value>(U("returnMessage"), obj);

    }).get();

    ASSERT_EQ(test.serialize(), U("\"test\""));
}

TEST(hub_connection_tests, send_message_after_connection_restart)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url + U("SignalR"));
    auto message = std::make_shared<utility::string_t>();
    auto received_event = std::make_shared<pplx::event>();

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_proxy.on(U("displayMessage"), [message, received_event](const web::json::value& arguments)
    {
        *message = arguments.serialize();
        received_event->set();
    });

    hub_conn->start().get();

    hub_conn->stop().get();

    hub_conn->start().then([&hub_proxy]()
    {
        web::json::value obj{};
        obj[0] = web::json::value(U("test"));

        return hub_proxy.invoke<web::json::value>(U("displayMessage"), obj);

    }).get();

    ASSERT_FALSE(received_event->wait(2000));

    ASSERT_EQ(*message, U("[\"Send: test\"]"));
}

TEST(hub_connection_tests, send_message_after_reconnect)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url + U("SignalR"));
    auto message = std::make_shared<utility::string_t>();
    auto reconnected_event = std::make_shared<pplx::event>();
    auto received_event = std::make_shared<pplx::event>();

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_conn->set_reconnected([reconnected_event]()
    {
        reconnected_event->set();
    });

    hub_proxy.on(U("displayMessage"), [message, received_event](const web::json::value& arguments)
    {
        *message = arguments.serialize();
        received_event->set();
    });

    hub_conn->start().get();

    try
    {
        hub_proxy.invoke<web::json::value>(U("forceReconnect")).get();
    }
    catch (...)
    {
    }

    ASSERT_FALSE(reconnected_event->wait(2000));

    web::json::value obj{};
    obj[0] = web::json::value(U("test"));

    hub_proxy.invoke<web::json::value>(U("displayMessage"), obj).get();

    ASSERT_FALSE(received_event->wait(2000));

    ASSERT_EQ(*message, U("[\"Send: test\"]"));
}