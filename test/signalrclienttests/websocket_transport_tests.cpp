// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "test_utils.h"
#include "signalrclient\trace_log_writer.h"
#include "test_websocket_client.h"
#include "websocket_transport.h"
#include "memory_log_writer.h"

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

    websocket_transport ws_transport{ client, connection_impl::create(_XPLATSTR("http://fake.uri"), _XPLATSTR(""),
        trace_level::none, std::make_shared<trace_log_writer>()) };

    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).get();

    ASSERT_TRUE(connect_called);
    ASSERT_TRUE(receive_called);
}

TEST(websocket_transport_connect, connect_propagates_exceptions)
{
    auto client = std::make_shared<test_websocket_client>();
    client->set_connect_function([](const web::uri &)->pplx::task<void>
    {
        return pplx::task_from_exception<void>(web_sockets::client::websocket_exception(_XPLATSTR("connecting failed")));
    });

    websocket_transport ws_transport{ client, connection_impl::create(_XPLATSTR("http://fake.uri"),
        _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>()) };

    try
    {
        ws_transport.connect(_XPLATSTR("http://fakeuri.org")).get();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::exception &e)
    {
        ASSERT_EQ(_XPLATSTR("connecting failed"), utility::conversions::to_string_t(e.what()));
    }
}

TEST(websocket_transport_connect, connect_logs_exceptions)
{
    auto client = std::make_shared<test_websocket_client>();
    client->set_connect_function([](const web::uri &) -> pplx::task<void>
    {
        return pplx::task_from_exception<void>(web_sockets::client::websocket_exception(_XPLATSTR("connecting failed")));
    });

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    websocket_transport ws_transport{ client, connection_impl::create(_XPLATSTR("http://fake.uri"), 
        _XPLATSTR(""), trace_level::messages, writer) };

    try
    {
        ws_transport.connect(_XPLATSTR("http://fakeuri.org")).wait();
    }
    catch (...)
    { }

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();

    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);

    ASSERT_EQ(
        _XPLATSTR("[websocket transport] exception when connecting to the server: connecting failed\n"),
        entry);
}

TEST(websocket_transport_connect, cannot_call_connect_on_already_connected_transport)
{
    auto client = std::make_shared<test_websocket_client>();
    websocket_transport ws_transport{ client, connection_impl::create(_XPLATSTR("http://fake.uri"),
        _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>()) };

    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).wait();

    try
    {
        ws_transport.connect(_XPLATSTR("http://fakeuri.org")).wait();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::exception &e)
    {
        ASSERT_EQ(_XPLATSTR("transport already connected"), utility::conversions::to_string_t(e.what()));
    }
}

TEST(websocket_transport_connect, can_connect_after_disconnecting)
{

    auto client = std::make_shared<test_websocket_client>();
    websocket_transport ws_transport{ client, connection_impl::create(_XPLATSTR("http://fake.uri"),
        _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>()) };

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

    websocket_transport ws_transport{ client, connection_impl::create(_XPLATSTR("http://fake.uri"),
        _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>()) };

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

    websocket_transport ws_transport{ client, connection_impl::create(_XPLATSTR("http://fake.uri"),
        _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>()) };

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

    websocket_transport ws_transport{ client, connection_impl::create(_XPLATSTR("http://fake.uri"),
        _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>()) };
    ws_transport.disconnect().get();
}

TEST(websocket_transport_disconnect, disconnect_logs_exceptions)
{
    auto client = std::make_shared<test_websocket_client>();
    client->set_close_function([]()->pplx::task<void>
    {
        return pplx::task_from_exception<void>(web_sockets::client::websocket_exception(_XPLATSTR("connection closing failed")));
    });

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    websocket_transport ws_transport{client, connection_impl::create(_XPLATSTR("http://fake.uri"), 
        _XPLATSTR(""), trace_level::messages, writer)};

    try
    {
        ws_transport.disconnect().get();
    }
    catch (...) 
    {}

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();

    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);

    ASSERT_EQ(
        _XPLATSTR("[websocket transport] exception when closing websocket: connection closing failed\n"),
        entry);
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

    websocket_transport ws_transport{ client, connection_impl::create(_XPLATSTR("http://fake.uri"),
        _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>()) };

    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).get();
    ws_transport.disconnect().get();

    receive_task_tce = pplx::task_completion_event<std::string>();
    ws_transport.connect(_XPLATSTR("http://fakeuri.org")).get();
    ws_transport.disconnect().get();

    ASSERT_EQ(2, num_called);
}

template<class T>
void receive_loop_logs_exception_runner(const T& e, const utility::string_t& expected_message);

TEST(websocket_transport_receive_loop, receive_loop_logs_websocket_exceptions)
{
    receive_loop_logs_exception_runner(
        web_sockets::client::websocket_exception(_XPLATSTR("receive failed")),
        _XPLATSTR("[websocket transport] websocket exception when receiving data: receive failed\n"));
}

TEST(websocket_transport_receive_loop, receive_loop_logs_if_receive_task_cancelled)
{
    receive_loop_logs_exception_runner(
        pplx::task_canceled("cancelled"),
        _XPLATSTR("[websocket transport] receive task cancelled: cancelled\n"));
}

TEST(websocket_transport_receive_loop, receive_loop_logs_std_exception)
{
    receive_loop_logs_exception_runner(
        std::exception("exception"), 
        _XPLATSTR("[websocket transport] error receiving response from websocket: exception\n"));
}

template<class T>
void receive_loop_logs_exception_runner(const T& e, const utility::string_t& expected_message)
{
    pplx::event receive_event;
    auto client = std::make_shared<test_websocket_client>();

    client->set_receive_function([&receive_event, &e]()->pplx::task<std::string>
    {
        receive_event.set();
        return pplx::task_from_exception<std::string>(e);
    });

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    websocket_transport ws_transport{ client, connection_impl::create(_XPLATSTR("http://fake.uri"),
        _XPLATSTR(""), trace_level::messages, writer) };

    ws_transport.connect(_XPLATSTR("url"))
        .then([&receive_event]()
    {
        receive_event.wait();
    }).get();

    // this is race'y but there is nothing we can block on
    pplx::wait(10);

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();

    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);

    ASSERT_EQ(expected_message, entry);
}