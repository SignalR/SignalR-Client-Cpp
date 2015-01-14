// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "test_utils.h"
#include "test_transport_factory.h"
#include "test_web_request_factory.h"
#include "hub_connection_impl.h"
#include "signalrclient\trace_log_writer.h"
#include "memory_log_writer.h"
#include "signalrclient\hub_exception.h"

using namespace signalr;

std::shared_ptr<hub_connection_impl> create_hub_connection(std::shared_ptr<websocket_client> websocket_client = create_test_websocket_client(),
    std::shared_ptr<log_writer> log_writer = std::make_shared<trace_log_writer>(), trace_level trace_level = trace_level::all)
{
    return hub_connection_impl::create( _XPLATSTR("http://fakeuri"), _XPLATSTR(""), trace_level, log_writer,
        create_test_web_request_factory(), std::make_unique<test_transport_factory>(websocket_client));
}

TEST(create_hub_proxy, create_hub_proxy_creates_proxy_with_correct_name)
{
    auto hub_proxy = create_hub_connection()->create_hub_proxy(_XPLATSTR("my_hub_proxy"));

    ASSERT_EQ(_XPLATSTR("my_hub_proxy"), hub_proxy->get_hub_name());
}

TEST(create_hub_proxy, create_hub_proxy_returns_existing_proxies_if_possible)
{
    auto hub_connection = create_hub_connection();
    auto hub_proxy_1 = hub_connection->create_hub_proxy(_XPLATSTR("my_hub_proxy"));
    auto hub_proxy_2 = hub_connection->create_hub_proxy(_XPLATSTR("my_hub_proxy"));

    ASSERT_EQ(hub_proxy_1.get(), hub_proxy_2.get());
}

TEST(create_hub_proxy, cannot_create_proxy_with_empty_name)
{
    try
    {
        create_hub_connection()->create_hub_proxy(_XPLATSTR(""));

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::invalid_argument& e)
    {
        ASSERT_STREQ("hub name cannot be empty", e.what());
    }
}

TEST(create_hub_proxy, cannot_create_proxy_if_connection_not_disconnected)
{
    try
    {
        auto websocket_client = create_test_websocket_client(
            /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
        auto hub_connection = create_hub_connection(websocket_client);

        hub_connection->start().get();

        hub_connection->create_hub_proxy(_XPLATSTR("myhub"));

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error& e)
    {
        ASSERT_STREQ("hub proxies cannot be created when the connection is not in the disconnected state", e.what());
    }
}

TEST(start, start_starts_connection)
{
    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
    auto hub_connection = create_hub_connection(websocket_client);

    hub_connection->start().get();

    ASSERT_EQ(connection_state::connected, hub_connection->get_connection_state());
}

TEST(start, start_sets_connection_data)
{
    web::uri requested_url;
    auto web_request_factory = std::make_unique<test_web_request_factory>([&requested_url](const web::uri &url)
    {
        requested_url = url;
        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)404, _XPLATSTR("Bad request"), _XPLATSTR("")));
    });

    auto hub_connection = hub_connection_impl::create(_XPLATSTR("http://fakeuri"), _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>(),
        std::move(web_request_factory), std::make_unique<test_transport_factory>(create_test_websocket_client()));
    hub_connection->create_hub_proxy(_XPLATSTR("my_hub"));
    hub_connection->create_hub_proxy(_XPLATSTR("your_hub"));

    try
    {
        hub_connection->start().get();
    }
    catch (...)
    {
    }

    ASSERT_EQ(web::uri(_XPLATSTR("http://fakeuri/negotiate?clientProtocol=1.4&connectionData=%5B%7B%22Name%22:%22my_hub%22%7D,%7B%22Name%22:%22your_hub%22%7D%5D")),
        requested_url);
}

TEST(start, start_logs_if_no_hub_proxies_exist_for_hub_connection)
{
    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
    auto hub_connection = create_hub_connection(websocket_client, writer, trace_level::info);

    hub_connection->start().get();

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);
    ASSERT_EQ(_XPLATSTR("[info        ] no hub proxies exist for this hub connection\n"), entry);
}

TEST(stop, stop_stops_connection)
{
    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
    auto hub_connection = create_hub_connection(websocket_client);

    hub_connection->start().get();
    hub_connection->stop().get();

    ASSERT_EQ(connection_state::disconnected, hub_connection->get_connection_state());
}

TEST(stop, connection_stopped_when_going_out_of_scope)
{
    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    {
        auto websocket_client = create_test_websocket_client(
            /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
        auto hub_connection = create_hub_connection(websocket_client, writer, trace_level::state_changes);

        hub_connection->start().get();
    }

    auto memory_writer = std::dynamic_pointer_cast<memory_log_writer>(writer);

    // The underlying connection_impl will be destroyed when the last reference to shared_ptr holding is released. This can happen
    // on a different thread in which case the dtor will be invoked on a different thread so we need to wait for this
    // to happen and if it does not the test will fail. There is nothing we can block on.
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

TEST(stop, stop_cancels_pending_callbacks)
{
    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{}"
        };

        if (call_number < 1)
        {
            call_number++;
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    hub_connection->start().get();
    auto t = hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
    hub_connection->stop();

    try
    {
        t.get();
        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error &e)
    {
        ASSERT_STREQ("\"connection was stopped before invocation result was received\"", e.what());
    }
}

TEST(stop, pending_callbacks_finished_if_hub_connections_goes_out_of_scope)
{
    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{}"
        };

        if (call_number < 1)
        {
            call_number++;
        }

        return pplx::task_from_result(responses[call_number]);
    });

    pplx::task<void> t;

    {
        auto hub_connection = create_hub_connection(websocket_client);
        hub_connection->start().get();
        t = hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
    }

    try
    {
        t.get();
        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error &e)
    {
        ASSERT_STREQ("\"connection went out of scope before invocation result was received\"", e.what());
    }
}

TEST(hub_invocation, hub_connection_invokes_users_code_on_hub_invocations)
{
    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number]()
    mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"C\":\"d- F430FB19\", \"M\" : [{\"H\":\"my_hub\", \"M\":\"broadcast\", \"A\" : [\"message\", 1]}] }",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    auto hub_proxy = hub_connection->create_hub_proxy(_XPLATSTR("my_hub"));

    auto payload = std::make_shared<utility::string_t>();
    auto on_broadcast_event = std::make_shared<pplx::event>();
    hub_proxy->on(_XPLATSTR("broadcast"), [on_broadcast_event, payload](const json::value& message)
    {
        *payload = message.serialize();
        on_broadcast_event->set();
    });

    hub_connection->start().get();
    ASSERT_FALSE(on_broadcast_event->wait(5000));

    ASSERT_EQ(_XPLATSTR("[\"message\",1]"), *payload);
}

TEST(hub_invocation, hub_connection_discards_persistent_connection_message_primitive_value)
{
    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{ \"C\":\"d-486F0DF9-BAO,5|BAV,1|BAW,0\", \"M\" : [\"Test\"] }",
            "{\"C\":\"d- F430FB19\", \"M\" : [{\"H\":\"my_hub\", \"M\":\"broadcast\", \"A\" : [\"signal event\", 1]}] }",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        return pplx::task_from_result(responses[call_number]);
    });

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());
    auto hub_connection = create_hub_connection(websocket_client, writer, trace_level::info);
    auto hub_proxy = hub_connection->create_hub_proxy(_XPLATSTR("my_hub"));

    auto on_broadcast_event = std::make_shared<pplx::event>();
    hub_proxy->on(_XPLATSTR("broadcast"), [on_broadcast_event](const json::value&)
    {
        on_broadcast_event->set();
    });

    hub_connection->start().get();
    ASSERT_FALSE(on_broadcast_event->wait(5000));

    auto memory_writer = std::dynamic_pointer_cast<memory_log_writer>(writer);
    auto log_entries = memory_writer->get_log_entries();
    ASSERT_TRUE(log_entries.size() >= 1);

    ASSERT_EQ(_XPLATSTR("[info        ] non-hub message received and will be discarded. message: \"Test\"\n"),
        remove_date_from_log_entry(log_entries[1]));
}

TEST(hub_invocation, hub_connection_invokes_persistent_connection_message_object)
{
    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{ \"C\":\"d-486F0DF9-BAO,5|BAV,1|BAW,0\", \"M\" : [{\"Name\": \"Test\"}] }",
            "{\"C\":\"d- F430FB19\", \"M\" : [{\"H\":\"my_hub\", \"M\":\"broadcast\", \"A\" : [\"signal event\", 1]}] }",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        return pplx::task_from_result(responses[call_number]);
    });

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());
    auto hub_connection = create_hub_connection(websocket_client, writer, trace_level::info);
    auto hub_proxy = hub_connection->create_hub_proxy(_XPLATSTR("my_hub"));

    auto on_broadcast_event = std::make_shared<pplx::event>();
    hub_proxy->on(_XPLATSTR("broadcast"), [on_broadcast_event](const json::value&)
    {
        on_broadcast_event->set();
    });

    hub_connection->start().get();
    ASSERT_FALSE(on_broadcast_event->wait(5000));

    auto memory_writer = std::dynamic_pointer_cast<memory_log_writer>(writer);
    auto log_entries = memory_writer->get_log_entries();
    ASSERT_TRUE(log_entries.size() >= 1);

    ASSERT_EQ(_XPLATSTR("[info        ] non-hub message received and will be discarded. message: {\"Name\":\"Test\"}\n"),
        remove_date_from_log_entry(log_entries[1]));
}

TEST(invoke, invoke_creates_correct_payload)
{
    utility::string_t payload;

    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); },
        /* send function */[&payload](const utility::string_t& m)
        {
            payload = m;
            return pplx::task_from_exception<void>(std::runtime_error("error"));
        });

    auto hub_connection = create_hub_connection(websocket_client);
    hub_connection->start().get();

    try
    {
        hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array()).get();
    }
    catch (...)
    {
        // the send is not setup to succeed because it's not needed in this test
    }

    ASSERT_EQ(_XPLATSTR("{\"A\":[],\"H\":\"my_hub\",\"I\":\"0\",\"M\":\"method\"}"), payload);
}

TEST(invoke, callback_not_called_if_send_throws)
{
    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); },
        /* send function */[](const utility::string_t&) { return pplx::task_from_exception<void>(std::runtime_error("error")); });

    auto hub_connection = create_hub_connection(websocket_client);
    hub_connection->start().get();

    try
    {
        hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array()).get();
        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error& e)
    {
        ASSERT_STREQ("error", e.what());
    }

    // stop completes all outstanting callbacks so if we did not remove a callback when `invoke_void` failed an
    // unobserved exception exception would be thrown. Note that this would happen on a different thread and would
    // crash the process
    hub_connection->stop().get();
}

TEST(hub_invocation, hub_connection_logs_if_no_hub_for_invocation)
{
    int call_number = -1;

    auto done_event = std::make_shared<pplx::event>();

    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, done_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"C\":\"d- F430FB19\", \"M\" : [{\"H\":\"my_hub\", \"M\":\"broadcast\", \"A\" : [\"message\", 1]}] }",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number == 2)
        {
            done_event->set();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());
    auto hub_connection = create_hub_connection(websocket_client, writer, trace_level::info);

    auto payload = std::make_shared<utility::string_t>();

    hub_connection->start().get();
    ASSERT_FALSE(done_event->wait(5000));

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_TRUE(log_entries.size() > 2);
    auto entry = remove_date_from_log_entry(log_entries[2]);
    ASSERT_EQ(_XPLATSTR("[info        ] no proxy found for hub invocation. hub: my_hub, method: broadcast\n"), entry);
}

TEST(invoke_json, invoke_returns_value_returned_from_the_server)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"I\":\"0\", \"R\" : \"abc\"}",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    auto result = hub_connection->start()
        .then([hub_connection, callback_registered_event]()
        {
            auto t = hub_connection->invoke_json(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
            callback_registered_event->set();
            return t;
        }).get();

    ASSERT_EQ(_XPLATSTR("\"abc\""), result.serialize());
}

TEST(invoke_json, invoke_propagates_errors_from_server_as_exceptions)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"I\":\"0\", \"E\" : \"Ooops\"}",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    try
    {
        hub_connection->start()
            .then([hub_connection, callback_registered_event]()
        {
            auto t = hub_connection->invoke_json(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
            callback_registered_event->set();
            return t;
        }).get();

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error& e)
    {
        ASSERT_STREQ("\"Ooops\"", e.what());
    }
}

TEST(invoke_json, invoke_propagates_hub_errors_from_server_as_hub_exceptions)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"I\":\"0\", \"E\" : \"Ooops\", \"H\": true, \"D\": { \"ErrorNumber\" : 42 }}",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    try
    {
        hub_connection->start()
            .then([hub_connection, callback_registered_event]()
        {
            auto t = hub_connection->invoke_json(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
            callback_registered_event->set();
            return t;
        }).get();

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const hub_exception& e)
    {
        ASSERT_STREQ("\"Ooops\"", e.what());
        ASSERT_EQ(_XPLATSTR("{\"ErrorNumber\":42}"), e.error_data().serialize());
    }
}

TEST(progress, progress_callback_called_for_progress_messages_json)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"C\":\"d-5E80A020-A,1|B,0|C,15|D,0\", \"M\":[{\"I\":\"P|1\", \"P\":{\"I\":\"0\", \"D\":1}}] }",
            "{\"C\":\"d-5E80A020-A,1|B,0|C,15|D,0\", \"M\":[{\"I\":\"P|1\", \"P\":{\"I\":\"0\", \"D\":2}}] }",
            "{\"I\":\"0\", \"R\":\"abc\"}",
            "{}"
        };

        call_number = min(call_number + 1, 4);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto progress_called_count = 0;
    auto progress_callback = [&progress_called_count](const json::value& data)
    {
        auto expected_value = ++progress_called_count;
        ASSERT_EQ(expected_value, data.as_integer());
    };

    auto hub_connection = create_hub_connection(websocket_client);
    hub_connection->start()
        .then([hub_connection, callback_registered_event, progress_callback]()
    {
        auto t = hub_connection->invoke_json(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array(), progress_callback);
        callback_registered_event->set();
        return t;
    }).get();

    ASSERT_EQ(2, progress_called_count);
}

TEST(invoke_void, invoke_unblocks_task_when_server_completes_call)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"I\":\"0\"}",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    hub_connection->start()
        .then([hub_connection, callback_registered_event]()
    {
        auto t = hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
        callback_registered_event->set();
        return t;
    }).get();

    // should not block
    ASSERT_TRUE(true);
}

TEST(invoke_void, invoke_logs_if_callback_for_given_id_not_found)
{
    auto message_received_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, message_received_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"I\":\"not tracked\"}",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number > 1)
        {
            message_received_event->set();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());
    auto hub_connection = create_hub_connection(websocket_client, writer, trace_level::info);
    hub_connection->start().get();

    ASSERT_FALSE(message_received_event->wait(5000));

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_TRUE(log_entries.size() > 1);

    auto entry = remove_date_from_log_entry(log_entries[2]);
    ASSERT_EQ(_XPLATSTR("[info        ] no callback found for id: not tracked\n"), entry);
}

TEST(invoke_void, invoke_propagates_errors_from_server_as_exceptions)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"I\":\"0\", \"E\" : \"Ooops\"}",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    try
    {
        hub_connection->start()
            .then([hub_connection, callback_registered_event]()
        {
            auto t = hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
            callback_registered_event->set();
            return t;
        }).get();

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error& e)
    {
        ASSERT_STREQ("\"Ooops\"", e.what());
    }
}

TEST(invoke_void, invoke_propagates_hub_errors_from_server_as_hub_exceptions)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"I\":\"0\", \"E\" : \"Ooops\", \"H\": true, \"D\": { \"ErrorNumber\" : 42 }}",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    try
    {
        hub_connection->start()
            .then([hub_connection, callback_registered_event]()
        {
            auto t = hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
            callback_registered_event->set();
            return t;
        }).get();

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const hub_exception& e)
    {
        ASSERT_STREQ("\"Ooops\"", e.what());
        ASSERT_EQ(_XPLATSTR("{\"ErrorNumber\":42}"), e.error_data().serialize());
    }
}

TEST(invoke_void, invoke_creates_hub_exception_even_if_no_error_data)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"I\":\"0\", \"E\" : \"Ooops\", \"H\": true }",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    try
    {
        hub_connection->start()
            .then([hub_connection, callback_registered_event]()
        {
            auto t = hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
            callback_registered_event->set();
            return t;
        }).get();

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const hub_exception& e)
    {
        ASSERT_STREQ("\"Ooops\"", e.what());
        ASSERT_TRUE(e.error_data().is_null());
    }
}

TEST(invoke_void, invoke_creates_runtime_error_even_hub_exception_indicator_false)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"I\":\"0\", \"E\" : \"Ooops\", \"H\": false }",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    try
    {
        hub_connection->start()
            .then([hub_connection, callback_registered_event]()
        {
            auto t = hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
            callback_registered_event->set();
            return t;
        }).get();

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error& e)
    {
        ASSERT_STREQ("\"Ooops\"", e.what());
        ASSERT_TRUE(dynamic_cast<const hub_exception *>(&e) == nullptr);
    }
}

TEST(invoke_void, invoke_creates_runtime_error_even_hub_exception_indicator_non_bool)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"I\":\"0\", \"E\" : \"Ooops\", \"H\": 42 }",
            "{}"
        };

        call_number = min(call_number + 1, 2);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto hub_connection = create_hub_connection(websocket_client);
    try
    {
        hub_connection->start()
            .then([hub_connection, callback_registered_event]()
        {
            auto t = hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array());
            callback_registered_event->set();
            return t;
        }).get();

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::runtime_error& e)
    {
        ASSERT_STREQ("\"Ooops\"", e.what());
        ASSERT_TRUE(dynamic_cast<const hub_exception *>(&e) == nullptr);
    }
}

TEST(progress, progress_callback_called_for_progress_messages)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"C\":\"d-5E80A020-A,1|B,0|C,15|D,0\", \"M\":[{\"I\":\"P|1\", \"P\":{\"I\":\"0\", \"D\":1}}] }",
            "{\"C\":\"d-5E80A020-A,1|B,0|C,15|D,0\", \"M\":[{\"I\":\"P|1\", \"P\":{\"I\":\"0\", \"D\":2}}] }",
            "{\"I\":\"0\"}",
            "{}"
        };

        call_number = min(call_number + 1, 4);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto progress_called_count = 0;
    auto progress_callback = [&progress_called_count](const json::value& data)
    {
        auto expected_value = ++progress_called_count;
        ASSERT_EQ(expected_value, data.as_integer());
    };

    auto hub_connection = create_hub_connection(websocket_client);
    hub_connection->start()
        .then([hub_connection, callback_registered_event, progress_callback]()
    {
        auto t = hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array(), progress_callback);
        callback_registered_event->set();
        return t;
    }).get();

    ASSERT_EQ(2, progress_called_count);
}

TEST(progress, exceptions_from_progress_callbacks_logged)
{
    auto callback_registered_event = std::make_shared<pplx::event>();

    int call_number = -1;
    auto websocket_client = create_test_websocket_client(
        /* receive function */ [call_number, callback_registered_event]()
        mutable {
        std::string responses[]
        {
            "{\"S\":1, \"M\":[] }",
            "{\"C\":\"d-5E80A020-A,1|B,0|C,15|D,0\", \"M\":[{\"I\":\"P|1\", \"P\":{\"I\":\"0\", \"D\":1}}] }",
            "{\"I\":\"0\"}",
            "{}"
        };

        call_number = min(call_number + 1, 3);

        if (call_number > 0)
        {
            callback_registered_event->wait();
        }

        return pplx::task_from_result(responses[call_number]);
    });

    auto progress_called_count = 0;
    auto progress_callback = [&progress_called_count](const json::value&)
    {
        throw std::runtime_error("bad stuff happened");
    };

    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());
    auto hub_connection = create_hub_connection(websocket_client, writer, trace_level::errors);

    hub_connection->start()
        .then([hub_connection, callback_registered_event, progress_callback]()
    {
        auto t = hub_connection->invoke_void(_XPLATSTR("my_hub"), _XPLATSTR("method"), json::value::array(), progress_callback);
        callback_registered_event->set();
        return t;
    }).get();

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);
    ASSERT_EQ(_XPLATSTR("[error       ] message_received callback threw an exception: bad stuff happened\n"), entry);
}