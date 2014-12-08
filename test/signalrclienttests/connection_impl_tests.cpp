// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "test_utils.h"
#include "test_web_request_factory.h"
#include "test_websocket_client.h"
#include "test_transport_factory.h"
#include "connection_impl.h"
#include "signalrclient\trace_level.h"
#include "signalrclient\trace_log_writer.h"
#include "web_request_factory.h"
#include "transport_factory.h"
#include "memory_log_writer.h"
#include "cpprest\ws_client.h"

using namespace signalr;
using namespace web::experimental;

static std::shared_ptr<connection_impl> create_connection(std::shared_ptr<websocket_client> websocket_client = std::make_shared<test_websocket_client>(),
    std::shared_ptr<log_writer> log_writer = std::make_shared<trace_log_writer>(), trace_level trace_level = trace_level::all)
{
    auto web_request_factory = std::make_unique<test_web_request_factory>([](const web::uri& url)
    {
        auto response_body =
            url.path() == _XPLATSTR("/negotiate")
            ? _XPLATSTR("{\"Url\":\"/signalr\", \"ConnectionToken\" : \"A==\", \"ConnectionId\" : \"f7707523-307d-4cba-9abf-3eef701241e8\", ")
            _XPLATSTR("\"KeepAliveTimeout\" : 20.0, \"DisconnectTimeout\" : 30.0, \"ConnectionTimeout\" : 110.0, \"TryWebSockets\" : true, ")
            _XPLATSTR("\"ProtocolVersion\" : \"1.4\", \"TransportConnectTimeout\" : 5.0, \"LongPollDelay\" : 0.0}")
            : url.path() == _XPLATSTR("/start")
                ? _XPLATSTR("{\"Response\":\"started\" }")
                : _XPLATSTR("");

        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });

    return connection_impl::create(_XPLATSTR("http://fakeuri"), _XPLATSTR(""), trace_level, log_writer,
        std::move(web_request_factory), std::make_unique<test_transport_factory>(websocket_client));
}

static std::shared_ptr<websocket_client> create_test_websocket_client(
    std::function<pplx::task<std::string>()> receive_function = [](){ return pplx::task_from_result<std::string>(""); },
    std::function<pplx::task<void>(const utility::string_t &msg)> send_function = [](const utility::string_t msg){ return pplx::task_from_result(); },
    std::function<pplx::task<void>(const web::uri &url)> connect_function = [](const web::uri &){ return pplx::task_from_result(); },
    std::function<pplx::task<void>()> close_function = [](){ return pplx::task_from_result(); })
{
    auto websocket_client = std::make_shared<test_websocket_client>();
    websocket_client->set_receive_function(receive_function);
    websocket_client->set_send_function(send_function);
    websocket_client->set_connect_function(connect_function);
    websocket_client->set_close_function(close_function);

    return websocket_client;
}

TEST(connection_impl_connection_state, initial_connection_state_is_disconnected)
{
    auto connection =
        connection_impl::create(_XPLATSTR("url"), _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>());

    ASSERT_EQ(connection_state::disconnected, connection->get_connection_state());
}

TEST(connection_impl_start, cannot_start_non_disconnected_exception)
{
    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
    auto connection = create_connection(websocket_client);

    connection->start().wait();

    try
    {
        connection->start().get();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::runtime_error& e)
    {
        ASSERT_STREQ("cannot start a connection that is not in the disconnected state", e.what());
    }
}

TEST(connection_impl_start, connection_state_is_connecting_when_connection_is_being_started)
{
    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_exception<std::string>(std::runtime_error("should not be invoked")); },
        /* send function */ [](const utility::string_t){ return pplx::task_from_exception<void>(std::runtime_error("should not be invoked"));  },
        /* connect function */[](const web::uri&)
        {
            return pplx::task_from_exception<void>(web_sockets::client::websocket_exception(_XPLATSTR("connecting failed")));
        });

    auto connection = create_connection(websocket_client, writer, trace_level::errors);

    connection->start()
        // this test is not set up to connect successfully so we have to observe exceptions otherwise
        // other tests may fail due to an unobserved exception from the outstanding start task
        .then([](pplx::task<void> start_task)
        {
            try
            {
                start_task.get();
            }
            catch (...)
            { }
        });

    ASSERT_EQ(connection->get_connection_state(), connection_state::connecting);
}

TEST(connection_impl_start, connection_state_is_connected_when_connection_established_succesfully)
{
    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
    auto connection = create_connection(websocket_client);
    connection->start().get();
    ASSERT_EQ(connection->get_connection_state(), connection_state::connected);
}

TEST(connection_impl_start, connection_state_is_disconnected_when_connection_cannot_be_established)
{
    auto web_request_factory = std::make_unique<test_web_request_factory>([](const web::uri &) -> std::unique_ptr<web_request>
    {
        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)404, _XPLATSTR("Bad request"), _XPLATSTR("")));
    });

    auto connection =
        connection_impl::create(_XPLATSTR("url"), _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>(),
        std::move(web_request_factory), std::make_unique<transport_factory>());

    try
    {
        connection->start().get();
    }
    catch (...)
    { }

    ASSERT_EQ(connection->get_connection_state(), connection_state::disconnected);
}

TEST(connection_impl_start, start_logs_exceptions)
{
    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    auto web_request_factory = std::make_unique<test_web_request_factory>([](const web::uri &) -> std::unique_ptr<web_request>
    {
        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)404, _XPLATSTR("Bad request"), _XPLATSTR("")));
    });

    auto connection =
        connection_impl::create(_XPLATSTR("url"), _XPLATSTR(""), trace_level::errors, writer,
            std::move(web_request_factory), std::make_unique<transport_factory>());

    try
    {
        connection->start().get();
    }
    catch (...)
    { }

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);
    ASSERT_EQ(_XPLATSTR("[error       ] connection could not be started due to: web exception - 404 Bad request\n"), entry);
}

TEST(connection_impl_start, start_propagates_exceptions_from_negotiate)
{
    auto web_request_factory = std::make_unique<test_web_request_factory>([](const web::uri &) -> std::unique_ptr<web_request>
    {
        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)404, _XPLATSTR("Bad request"), _XPLATSTR("")));
    });

    auto connection =
        connection_impl::create(_XPLATSTR("url"), _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>(),
        std::move(web_request_factory), std::make_unique<transport_factory>());

    try
    {
        connection->start().get();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::exception &e)
    {
        ASSERT_EQ(_XPLATSTR("web exception - 404 Bad request"), utility::conversions::to_string_t(e.what()));
    }
}

TEST(connection_impl_start, start_fails_if_transport_connect_throws)
{
    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_exception<std::string>(std::runtime_error("should not be invoked")); },
        /* send function */ [](const utility::string_t){ return pplx::task_from_exception<void>(std::runtime_error("should not be invoked"));  },
        /* connect function */[](const web::uri&)
        {
            return pplx::task_from_exception<void>(web_sockets::client::websocket_exception(_XPLATSTR("connecting failed")));
        }
    );

    auto connection = create_connection(websocket_client, writer, trace_level::errors);

    try
    {
        connection->start().get();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::exception &e)
    {
        ASSERT_EQ(_XPLATSTR("connecting failed"), utility::conversions::to_string_t(e.what()));
    }

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_TRUE(log_entries.size() > 1);

    auto entry = remove_date_from_log_entry(log_entries[1]);
    ASSERT_EQ(_XPLATSTR("[error       ] transport could not connect due to: connecting failed\n"), entry);
}

TEST(connection_impl_start, start_fails_if_TryWebsockets_false_and_no_fallback_transport)
{
    auto web_request_factory = std::make_unique<test_web_request_factory>([](const web::uri &) -> std::unique_ptr<web_request>
    {
        utility::string_t response_body(
            _XPLATSTR("{\"Url\":\"/signalr\", \"ConnectionToken\" : \"A==\", \"ConnectionId\" : \"f7707523-307d-4cba-9abf-3eef701241e8\", ")
            _XPLATSTR("\"KeepAliveTimeout\" : 20.0, \"DisconnectTimeout\" : 30.0, \"ConnectionTimeout\" : 110.0, \"TryWebSockets\" : false, ")
            _XPLATSTR("\"ProtocolVersion\" : \"1.4\", \"TransportConnectTimeout\" : 5.0, \"LongPollDelay\" : 0.0}"));

        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });

    auto websocket_client = std::make_shared<test_websocket_client>();
    auto connection =
        connection_impl::create(_XPLATSTR("url"), _XPLATSTR(""), trace_level::errors, std::make_shared<trace_log_writer>(),
        std::move(web_request_factory), std::make_unique<test_transport_factory>(websocket_client));

    try
    {
        connection->start().get();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::exception &e)
    {
        ASSERT_EQ(_XPLATSTR("websockets not supported on the server and there is no fallback transport"),
            utility::conversions::to_string_t(e.what()));
    }
}

TEST(connection_impl_start, start_fails_if_start_request_fails)
{
    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    auto web_request_factory = std::make_unique<test_web_request_factory>([](const web::uri& url)
    {
        auto response_body =
            url.path() == _XPLATSTR("/negotiate")
            ? _XPLATSTR("{\"Url\":\"/signalr\", \"ConnectionToken\" : \"A==\", \"ConnectionId\" : \"f7707523-307d-4cba-9abf-3eef701241e8\", ")
            _XPLATSTR("\"KeepAliveTimeout\" : 20.0, \"DisconnectTimeout\" : 30.0, \"ConnectionTimeout\" : 110.0, \"TryWebSockets\" : true, ")
            _XPLATSTR("\"ProtocolVersion\" : \"1.4\", \"TransportConnectTimeout\" : 5.0, \"LongPollDelay\" : 0.0}")
            : _XPLATSTR("{ }");

        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });

    auto websocket_client = std::make_shared<test_websocket_client>();
    websocket_client->set_receive_function([]()->pplx::task<std::string>
    {
        return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }"));
    });

    auto connection =
        connection_impl::create(_XPLATSTR("http://fakeuri"), _XPLATSTR(""), trace_level::messages, writer,
        std::move(web_request_factory), std::make_unique<test_transport_factory>(websocket_client));

    try
    {
        connection->start().get();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::runtime_error &e)
    {
        ASSERT_STREQ("start request failed due to unexpected response from the server: { }", e.what());
    }
}

TEST(connection_impl_process_response, process_response_logs_messages)
{
    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());
    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
    auto connection = create_connection(websocket_client, writer, trace_level::messages);

    connection->start().get();

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);
    ASSERT_EQ(_XPLATSTR("[message     ] processing message: {\"S\":1, \"M\":[] }\n"), entry);
}

TEST(connection_impl_send, message_sent)
{
    utility::string_t actual_message;

    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); },
        /* send function */ [&actual_message](const utility::string_t& message)
    {
        actual_message = message;
        return pplx::task_from_result();
    });

    auto connection = create_connection(websocket_client);

    const utility::string_t message{ _XPLATSTR("Test message") };

    connection->start()
        .then([connection, message]()
        {
            return connection->send(message);
        }).get();

    ASSERT_EQ(message, actual_message);
}

TEST(connection_impl_send, send_throws_if_connection_not_connected)
{
    auto connection =
        connection_impl::create(_XPLATSTR("url"), _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>());

    try
    {
        connection->send(_XPLATSTR("whatever")).get();
        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error &e)
    {
        ASSERT_STREQ("cannot send data when the connection is not in the connected state. current connection state: disconnected", e.what());
    }
}

TEST(connection_impl_send, exceptions_from_send_logged_and_propagated)
{
    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); },
        /* send function */ [](const utility::string_t&){ return pplx::task_from_exception<void>(std::runtime_error("error")); });

    auto connection = create_connection(websocket_client, writer, trace_level::errors);

    try
    {
        connection->start()
            .then([connection]()
        {
            return connection->send(_XPLATSTR("Test message"));
        }).get();

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error &e)
    {
        ASSERT_STREQ("error", e.what());
    }

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);
    ASSERT_EQ(_XPLATSTR("[error       ] error sending data: error\n"), entry);
}

TEST(connection_impl_set_message_received, callback_invoked_when_message_received)
{
    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{ \"C\":\"d-486F0DF9-BAO,5|BAV,1|BAW,0\", \"M\" : [\"Test\"] }",
            "{ \"C\":\"d-486F0DF9-BAO,5|BAV,1|BAW,0\", \"M\" : [\"release\"] }",
            "{}"
        };

        call_number = min(call_number + 1, 3);

        return pplx::task_from_result(responses[call_number]);
    });

    auto connection = create_connection(websocket_client);

    auto message = std::make_shared<utility::string_t>();

    auto message_received_event = std::make_shared<pplx::event>();
    connection->set_message_received([message, message_received_event](const utility::string_t &m){
        if (m == _XPLATSTR("\"Test\""))
        {
            *message = m;
        }

        if (m == _XPLATSTR("\"release\""))
        {
            message_received_event->set();
        }
    });

    connection->start().get();

    ASSERT_FALSE(message_received_event->wait(5000));

    ASSERT_EQ(_XPLATSTR("\"Test\""), *message);
}

TEST(connection_impl_set_message_received, exception_from_callback_caught_and_logged)
{
    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{ \"C\":\"d-486F0DF9-BAO,5|BAV,1|BAW,0\", \"M\" : [\"throw\"] }",
            "{ \"C\":\"d-486F0DF9-BAO,5|BAV,1|BAW,0\", \"M\" : [\"release\"] }",
            "{}"
        };

        call_number = min(call_number + 1, 3);

        return pplx::task_from_result(responses[call_number]);
    });

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());
    auto connection = create_connection(websocket_client, writer, trace_level::errors);

    auto message_received_event = std::make_shared<pplx::event>();
    connection->set_message_received([message_received_event](const utility::string_t &m){
        if (m == _XPLATSTR("\"throw\""))
        {
            throw std::runtime_error("oops");
        }

        if (m == _XPLATSTR("\"release\""))
        {
            message_received_event->set();
        }
    });

    connection->start().get();

    ASSERT_FALSE(message_received_event->wait(5000));

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);
    ASSERT_EQ(_XPLATSTR("[error       ] message_received callback threw an exception: oops\n"), entry);
}

TEST(connection_impl_set_message_received, non_std_exception_from_callback_caught_and_logged)
{
    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{ \"C\":\"d-486F0DF9-BAO,5|BAV,1|BAW,0\", \"M\" : [\"throw\"] }",
            "{ \"C\":\"d-486F0DF9-BAO,5|BAV,1|BAW,0\", \"M\" : [\"release\"] }",
            "{}"
        };

        call_number = min(call_number + 1, 3);

        return pplx::task_from_result(responses[call_number]);
    });

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());
    auto connection = create_connection(websocket_client, writer, trace_level::errors);

    auto message_received_event = std::make_shared<pplx::event>();
    connection->set_message_received([message_received_event](const utility::string_t &m)
    {
        if (m == _XPLATSTR("\"throw\""))
        {
            throw 42;
        }

        if (m == _XPLATSTR("\"release\""))
        {
            message_received_event->set();
        }
    });

    connection->start().get();

    ASSERT_FALSE(message_received_event->wait(5000));

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);
    ASSERT_EQ(_XPLATSTR("[error       ] message_received callback threw an unknown exception.\n"), entry);
}

TEST(connection_impl_set_message_received, error_logged_for_malformed_payload)
{
    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "42",
            "{ \"C\":\"d-486F0DF9-BAO,5|BAV,1|BAW,0\", \"M\" : [\"release\"] }",
            "{}"
        };

        call_number = min(call_number + 1, 3);

        return pplx::task_from_result(responses[call_number]);
    });

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());
    auto connection = create_connection(websocket_client, writer, trace_level::errors);

    auto message_received_event = std::make_shared<pplx::event>();
    connection->set_message_received([message_received_event](const utility::string_t&)
    {
        // this is called only once because we have just one response with a message
        message_received_event->set();
    });

    connection->start().get();

    ASSERT_FALSE(message_received_event->wait(5000));

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);
    ASSERT_EQ(_XPLATSTR("[error       ] error occured when parsing response: not an object. response: 42\n"), entry);
}

TEST(connection_impl_set_message_received, callback_can_be_set_only_in_disconnected_state)
{
    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
    auto connection = create_connection(websocket_client);

    connection->start().get();

    try
    {
        connection->set_message_received([](const utility::string_t&){});
        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error &e)
    {
        ASSERT_STREQ("cannot set the callback when the connection is not in the disconnected state. current connection state: connected", e.what());
    }
}

TEST(connection_impl_stop, stopping_disconnected_connection_is_no_op)
{
    std::shared_ptr<log_writer> writer{ std::make_shared<memory_log_writer>() };
    auto connection = connection_impl::create(_XPLATSTR("http://fakeuri"), _XPLATSTR(""), trace_level::all, writer);
    connection->stop().get();

    ASSERT_EQ(connection_state::disconnected, connection->get_connection_state());

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_TRUE(log_entries.empty());
}

TEST(connection_impl_stop, stopping_disconnecting_connection_returns_cancelled_task)
{
    pplx::event close_event;
    auto writer = std::shared_ptr<log_writer>{std::make_shared<memory_log_writer>()};

    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); },
        /* send function */ [](const utility::string_t){ return pplx::task_from_exception<void>(std::runtime_error("should not be invoked")); },
        /* connect function */ [&close_event](const web::uri&) { return pplx::task_from_result(); },
        /* close function */ [&close_event]()
        {
            return pplx::create_task([&close_event]()
            {
                close_event.wait();
            });
        });

    auto connection = create_connection(websocket_client, writer, trace_level::state_changes);

    connection->start().get();
    auto stop_task = connection->stop();

    try
    {
        connection->stop().get();
        ASSERT_FALSE(true); // exception expected but not thrown
    }
    catch (const pplx::task_canceled&)
    { }

    close_event.set();
    stop_task.get();

    ASSERT_EQ(connection_state::disconnected, connection->get_connection_state());

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_EQ(4, log_entries.size());
    ASSERT_EQ(_XPLATSTR("[state change] disconnected -> connecting\n"), remove_date_from_log_entry(log_entries[0]));
    ASSERT_EQ(_XPLATSTR("[state change] connecting -> connected\n"), remove_date_from_log_entry(log_entries[1]));
    ASSERT_EQ(_XPLATSTR("[state change] connected -> disconnecting\n"), remove_date_from_log_entry(log_entries[2]));
    ASSERT_EQ(_XPLATSTR("[state change] disconnecting -> disconnected\n"), remove_date_from_log_entry(log_entries[3]));
}

TEST(connection_impl_stop, can_start_and_stop_connection)
{
    auto writer = std::shared_ptr<log_writer>{std::make_shared<memory_log_writer>()};

    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
    auto connection = create_connection(websocket_client, writer, trace_level::state_changes);

    connection->start()
        .then([connection]()
        {
            return connection->stop();
        }).get();

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_EQ(4, log_entries.size());
    ASSERT_EQ(_XPLATSTR("[state change] disconnected -> connecting\n"), remove_date_from_log_entry(log_entries[0]));
    ASSERT_EQ(_XPLATSTR("[state change] connecting -> connected\n"), remove_date_from_log_entry(log_entries[1]));
    ASSERT_EQ(_XPLATSTR("[state change] connected -> disconnecting\n"), remove_date_from_log_entry(log_entries[2]));
    ASSERT_EQ(_XPLATSTR("[state change] disconnecting -> disconnected\n"), remove_date_from_log_entry(log_entries[3]));
}

TEST(connection_impl_stop, can_start_and_stop_connection_multiple_times)
{
    auto writer = std::shared_ptr<log_writer>{std::make_shared<memory_log_writer>()};

    {
        auto websocket_client = create_test_websocket_client(
            /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
        auto connection = create_connection(websocket_client, writer, trace_level::state_changes);

        connection->start()
            .then([connection]()
        {
            return connection->stop();
        })
        .then([connection]()
        {
            return connection->start();
        }).get();
    }

    auto memory_writer = std::dynamic_pointer_cast<memory_log_writer>(writer);

    // The connection_impl will be destroyed when the last reference to shared_ptr holding is released. This can happen
    // on a different thread in which case the dtor will be invoked on a different thread so we need to wait for this
    // to happen and if it does not the test will fail
    for (int wait_time_ms = 5; wait_time_ms < 100 && memory_writer->get_log_entries().size() < 8; wait_time_ms <<= 1)
    {
        pplx::wait(wait_time_ms);
    }

    auto log_entries = memory_writer->get_log_entries();
    ASSERT_EQ(8, log_entries.size());
    ASSERT_EQ(_XPLATSTR("[state change] disconnected -> connecting\n"), remove_date_from_log_entry(log_entries[0]));
    ASSERT_EQ(_XPLATSTR("[state change] connecting -> connected\n"), remove_date_from_log_entry(log_entries[1]));
    ASSERT_EQ(_XPLATSTR("[state change] connected -> disconnecting\n"), remove_date_from_log_entry(log_entries[2]));
    ASSERT_EQ(_XPLATSTR("[state change] disconnecting -> disconnected\n"), remove_date_from_log_entry(log_entries[3]));
    ASSERT_EQ(_XPLATSTR("[state change] disconnected -> connecting\n"), remove_date_from_log_entry(log_entries[4]));
    ASSERT_EQ(_XPLATSTR("[state change] connecting -> connected\n"), remove_date_from_log_entry(log_entries[5]));
    ASSERT_EQ(_XPLATSTR("[state change] connected -> disconnecting\n"), remove_date_from_log_entry(log_entries[6]));
    ASSERT_EQ(_XPLATSTR("[state change] disconnecting -> disconnected\n"), remove_date_from_log_entry(log_entries[7]));
}

TEST(connection_impl_stop, dtor_stops_the_connection)
{
    auto writer = std::shared_ptr<log_writer>{std::make_shared<memory_log_writer>()};

    {
        auto websocket_client = create_test_websocket_client(
            /* receive function */ []() { pplx::wait(1); return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
        auto connection = create_connection(websocket_client, writer, trace_level::state_changes);

        connection->start().get();
    }

    auto memory_writer = std::dynamic_pointer_cast<memory_log_writer>(writer);

    // The connection_impl will be destroyed when the last reference to shared_ptr holding is released. This can happen
    // on a different thread in which case the dtor will be invoked on a different thread so we need to wait for this
    // to happen and if it does not the test will fail
    for (int wait_time_ms = 5; wait_time_ms < 100 && memory_writer->get_log_entries().size() < 4; wait_time_ms <<= 1)
    {
        pplx::wait(wait_time_ms);
    }

    auto log_entries = memory_writer->get_log_entries();
    ASSERT_EQ(4, log_entries.size());
    ASSERT_EQ(_XPLATSTR("[state change] disconnected -> connecting\n"), remove_date_from_log_entry(log_entries[0]));
    ASSERT_EQ(_XPLATSTR("[state change] connecting -> connected\n"), remove_date_from_log_entry(log_entries[1]));
    ASSERT_EQ(_XPLATSTR("[state change] connected -> disconnecting\n"), remove_date_from_log_entry(log_entries[2]));
    ASSERT_EQ(_XPLATSTR("[state change] disconnecting -> disconnected\n"), remove_date_from_log_entry(log_entries[3]));
}

TEST(connection_impl_stop, stop_cancels_ongoing_start_request)
{
    auto disconnect_completed_event = std::make_shared<pplx::event>();

    auto websocket_client = create_test_websocket_client(
        /* receive function */ [disconnect_completed_event]()
        {
            disconnect_completed_event->wait();
            return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }"));
        });

    auto writer = std::shared_ptr<log_writer>{std::make_shared<memory_log_writer>()};
    auto connection = create_connection(std::make_shared<test_websocket_client>(), writer, trace_level::all);

    auto start_task = connection->start();
    connection->stop().get();
    disconnect_completed_event->set();

    start_task.then([](pplx::task<void> t)
    {
        try
        {
            t.get();
            ASSERT_TRUE(false); // exception expected but not thrown
        }
        catch (const pplx::task_canceled &)
        { }
    }).get();

    ASSERT_EQ(connection_state::disconnected, connection->get_connection_state());

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_EQ(3, log_entries.size());
    ASSERT_EQ(_XPLATSTR("[state change] disconnected -> connecting\n"), remove_date_from_log_entry(log_entries[0]));
    ASSERT_EQ(_XPLATSTR("[info        ] starting the connection has been cancelled.\n"), remove_date_from_log_entry(log_entries[1]));
    ASSERT_EQ(_XPLATSTR("[state change] connecting -> disconnected\n"), remove_date_from_log_entry(log_entries[2]));
}

TEST(connection_impl_stop, stop_ignores_exceptions_from_abort_requests)
{
    auto writer = std::shared_ptr<log_writer>{std::make_shared<memory_log_writer>()};

    auto web_request_factory = std::make_unique<test_web_request_factory>([](const web::uri& url)
    {
        auto response_body =
            url.path() == _XPLATSTR("/negotiate")
            ? _XPLATSTR("{\"Url\":\"/signalr\", \"ConnectionToken\" : \"A==\", \"ConnectionId\" : \"f7707523-307d-4cba-9abf-3eef701241e8\", ")
            _XPLATSTR("\"KeepAliveTimeout\" : 20.0, \"DisconnectTimeout\" : 30.0, \"ConnectionTimeout\" : 110.0, \"TryWebSockets\" : true, ")
            _XPLATSTR("\"ProtocolVersion\" : \"1.4\", \"TransportConnectTimeout\" : 5.0, \"LongPollDelay\" : 0.0}")
            : url.path() == _XPLATSTR("/start")
                ? _XPLATSTR("{\"Response\":\"started\" }")
                : _XPLATSTR("");

        return url.path() == _XPLATSTR("/abort")
            ? std::unique_ptr<web_request>(new web_request_stub((unsigned short)503, _XPLATSTR("Bad request"), response_body))
            : std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });

    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });

    auto connection =
        connection_impl::create(_XPLATSTR("http://fakeuri"), _XPLATSTR(""), trace_level::state_changes,
        writer, std::move(web_request_factory), std::make_unique<test_transport_factory>(websocket_client));

    connection->start()
        .then([connection]()
    {
        return connection->stop();
    }).get();

    ASSERT_EQ(connection_state::disconnected, connection->get_connection_state());

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_EQ(4, log_entries.size());
    ASSERT_EQ(_XPLATSTR("[state change] disconnected -> connecting\n"), remove_date_from_log_entry(log_entries[0]));
    ASSERT_EQ(_XPLATSTR("[state change] connecting -> connected\n"), remove_date_from_log_entry(log_entries[1]));
    ASSERT_EQ(_XPLATSTR("[state change] connected -> disconnecting\n"), remove_date_from_log_entry(log_entries[2]));
    ASSERT_EQ(_XPLATSTR("[state change] disconnecting -> disconnected\n"), remove_date_from_log_entry(log_entries[3]));
}

TEST(connection_impl_change_state, change_state_logs)
{
    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());
    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
    auto connection = create_connection(websocket_client, writer, trace_level::state_changes);

    connection->start().wait();

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);
    ASSERT_EQ(_XPLATSTR("[state change] disconnected -> connecting\n"), entry);
}