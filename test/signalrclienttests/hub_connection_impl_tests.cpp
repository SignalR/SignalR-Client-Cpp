// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "test_utils.h"
#include "test_transport_factory.h"
#include "hub_connection_impl.h"
#include "signalrclient\trace_log_writer.h"

using namespace signalr;

std::unique_ptr<hub_connection_impl> create_hub_connection(std::shared_ptr<websocket_client> websocket_client = create_test_websocket_client(),
    std::shared_ptr<log_writer> log_writer = std::make_shared<trace_log_writer>(), trace_level trace_level = trace_level::all)
{
    return std::make_unique<hub_connection_impl>( _XPLATSTR("http://fakeuri"), _XPLATSTR(""), trace_level, log_writer,
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

TEST(stop, stop_stops_connection)
{
    auto websocket_client = create_test_websocket_client(
        /* receive function */ []() { return pplx::task_from_result(std::string("{\"S\":1, \"M\":[] }")); });
    auto hub_connection = create_hub_connection(websocket_client);

    hub_connection->start().get();
    hub_connection->stop().get();

    ASSERT_EQ(connection_state::disconnected, hub_connection->get_connection_state());
}