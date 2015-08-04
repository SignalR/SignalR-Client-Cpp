// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "stdafx.h"
#include <string>
#include "cpprest/details/basic_types.h"
#include "cpprest/json.h"
#include "connection.h"
#include "hub_connection.h"

extern utility::string_t url;

TEST(hub_connection_tests, connection_status_start_stop_start_reconnect)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url);
    auto weak_hub_conn = std::weak_ptr<signalr::hub_connection>(hub_conn);
    auto reconnecting_event = std::make_shared<signalr::event>();
    auto reconnected_event = std::make_shared<signalr::event>();

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
    auto hub_conn = std::make_shared<signalr::hub_connection>(url + U("custom"), U(""), signalr::trace_level::all, nullptr, false);
    auto message = std::make_shared<utility::string_t>();
    auto received_event = std::make_shared<signalr::event>();

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_proxy.on(U("sendString"), [message, received_event](const web::json::value& arguments)
    {
        *message = arguments.serialize();
        received_event->set();
    });

    hub_conn->start().then([&hub_proxy]()
    {
        web::json::value obj{};
        obj[0] = web::json::value(U("test"));

        return hub_proxy.invoke<web::json::value>(U("invokeWithString"), obj);

    }).get();

    ASSERT_FALSE(received_event->wait(2000));

    ASSERT_EQ(*message, U("[\"Send: test\"]"));
}

TEST(hub_connection_tests, send_message_return)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url);

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    auto test = hub_conn->start().then([&hub_proxy]()
    {
        web::json::value obj{};
        obj[0] = web::json::value(U("test"));

        return hub_proxy.invoke<web::json::value>(U("returnString"), obj);

    }).get();

    ASSERT_EQ(test.serialize(), U("\"test\""));
}

TEST(hub_connection_tests, send_message_after_connection_restart)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url);
    auto message = std::make_shared<utility::string_t>();
    auto received_event = std::make_shared<signalr::event>();

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_proxy.on(U("sendString"), [message, received_event](const web::json::value& arguments)
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

        return hub_proxy.invoke<web::json::value>(U("invokeWithString"), obj);

    }).get();

    ASSERT_FALSE(received_event->wait(2000));

    ASSERT_EQ(*message, U("[\"Send: test\"]"));
}

TEST(hub_connection_tests, send_message_after_reconnect)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url);
    auto message = std::make_shared<utility::string_t>();
    auto reconnected_event = std::make_shared<signalr::event>();
    auto received_event = std::make_shared<signalr::event>();

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_conn->set_reconnected([reconnected_event]()
    {
        reconnected_event->set();
    });

    hub_proxy.on(U("sendString"), [message, received_event](const web::json::value& arguments)
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

    hub_proxy.invoke<web::json::value>(U("invokeWithString"), obj).get();

    ASSERT_FALSE(received_event->wait(2000));

    ASSERT_EQ(*message, U("[\"Send: test\"]"));
}

TEST(hub_connection_tests, send_message_empty_param)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url);
    auto message = std::make_shared<utility::string_t>();
    auto received_event = std::make_shared<signalr::event>();

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_proxy.on(U("sendString"), [message, received_event](const web::json::value& arguments)
    {
        *message = arguments.serialize();
        received_event->set();
    });

    hub_conn->start().then([&hub_proxy]()
    {
        return hub_proxy.invoke<web::json::value>(U("invokeWithEmptyParam"));

    }).get();

    ASSERT_FALSE(received_event->wait(2000));

    ASSERT_EQ(*message, U("[\"Send\"]"));
}

TEST(hub_connection_tests, send_message_primitive_params)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url);
    auto message = std::make_shared<utility::string_t>();
    auto received_event = std::make_shared<signalr::event>();

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_proxy.on(U("sendPrimitiveParams"), [message, received_event](const web::json::value& arguments)
    {
        *message = arguments.serialize();
        received_event->set();
    });

    hub_conn->start().then([&hub_proxy]()
    {
        web::json::value obj{};
        obj[0] = web::json::value(5);
        obj[1] = web::json::value(21.05);
        obj[2] = web::json::value(8.999999999);
        obj[3] = web::json::value(true);
        obj[4] = web::json::value('a');
        return hub_proxy.invoke<web::json::value>(U("invokeWithPrimitiveParams"), obj);

    }).get();

    ASSERT_FALSE(received_event->wait(2000));

    web::json::value obj{};
    obj[0] = web::json::value(6);
    obj[1] = web::json::value(22.05);
    obj[2] = web::json::value(9.999999999);
    obj[3] = web::json::value(true);
    obj[4] = web::json::value::string(U("a"));

    ASSERT_EQ(*message, obj.serialize());
}

TEST(hub_connection_tests, send_message_complex_type)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url);
    auto message = std::make_shared<utility::string_t>();
    auto received_event = std::make_shared<signalr::event>();

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_proxy.on(U("sendComplexType"), [message, received_event](const web::json::value& arguments)
    {
        *message = arguments.serialize();
        received_event->set();
    });

    hub_conn->start().then([&hub_proxy]()
    {
        web::json::value obj{};
        web::json::value person;
        web::json::value address;
        address[U("street")] = web::json::value::string(U("main st"));
        address[U("zip")] = web::json::value::string(U("98052"));
        person[U("address")] = address;
        person[U("name")] = web::json::value::string(U("test"));
        person[U("age")] = web::json::value::number(15);
        obj[0] = person;

        return hub_proxy.invoke<web::json::value>(U("invokeWithComplexType"), obj);

    }).get();

    ASSERT_FALSE(received_event->wait(2000));

    ASSERT_EQ(*message, U("[{\"Address\":{\"Street\":\"main st\",\"Zip\":\"98052\"},\"Age\":15,\"Name\":\"test\"}]"));
}

TEST(hub_connection_tests, send_message_complex_type_return)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url);

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    auto test = hub_conn->start().then([&hub_proxy]()
    {
        web::json::value obj{};
        web::json::value person;
        web::json::value address;
        address[U("street")] = web::json::value::string(U("main st"));
        address[U("zip")] = web::json::value::string(U("98052"));
        person[U("address")] = address;
        person[U("name")] = web::json::value::string(U("test"));
        person[U("age")] = web::json::value::number(15);
        obj[0] = person;

        return hub_proxy.invoke<web::json::value>(U("returnComplexType"), obj);

    }).get();

    ASSERT_EQ(test.serialize(), U("{\"Address\":{\"Street\":\"main st\",\"Zip\":\"98052\"},\"Age\":15,\"Name\":\"test\"}"));
}

TEST(hub_connection_tests, progress_report)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url);
    std::vector<int> vec;

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    hub_conn->start().then([&hub_proxy, &vec]()
    {
        return hub_proxy.invoke<web::json::value>(U("invokeWithProgress"), [&vec](const web::json::value& arguments)
        {
            vec.push_back(arguments.as_integer());
        });

    }).get();

    ASSERT_EQ(vec.size(), 5);

    for (size_t i = 0; i < vec.size(); i++)
    {
        ASSERT_EQ(vec[i], i);
    }
}

TEST(hub_connection_tests, progress_report_with_return)
{
    auto hub_conn = std::make_shared<signalr::hub_connection>(url);
    std::vector<int> vec;

    auto hub_proxy = hub_conn->create_hub_proxy(U("hubConnection"));

    auto test = hub_conn->start().then([&hub_proxy, &vec]()
    {
        web::json::value obj{};
        obj[0] = web::json::value(U("test"));

        return hub_proxy.invoke<web::json::value>(U("invokeWithProgress"), obj, [&vec](const web::json::value& arguments)
        {
            vec.push_back(arguments.as_integer());
        });

    }).get();

    ASSERT_EQ(test.serialize(), U("\"test done!\""));

    ASSERT_EQ(vec.size(), 5);

    for (size_t i = 0; i < vec.size(); i++)
    {
        ASSERT_EQ(vec[i], i);
    }
}
