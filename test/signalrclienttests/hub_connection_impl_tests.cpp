// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "hub_connection_impl.h"
#include "signalrclient\trace_log_writer.h"

using namespace signalr;

TEST(create_hub_proxy, create_hub_proxy_creates_proxy_with_correct_name)
{
    auto hub_proxy = hub_connection_impl{ trace_level::none, std::make_shared<trace_log_writer>() }
        .create_hub_proxy(utility::string_t(_XPLATSTR("my_hub_proxy")));

    ASSERT_EQ(_XPLATSTR("my_hub_proxy"), hub_proxy->get_hub_name());
}

TEST(create_hub_proxy, create_hub_proxy_returns_existing_proxies_if_possible)
{
    hub_connection_impl hub_connection{ trace_level::none, std::make_shared<trace_log_writer>() };
    auto hub_proxy_1 = hub_connection.create_hub_proxy(utility::string_t(_XPLATSTR("my_hub_proxy")));
    auto hub_proxy_2 = hub_connection.create_hub_proxy(utility::string_t(_XPLATSTR("my_hub_proxy")));

    ASSERT_EQ(hub_proxy_1.get(), hub_proxy_2.get());
}

TEST(create_hub_proxy, cannot_create_proxy_with_empty_name)
{
    try
    {
        hub_connection_impl{ trace_level::none, std::make_shared<trace_log_writer>() }
            .create_hub_proxy(utility::string_t(_XPLATSTR("")));

        ASSERT_TRUE(false); // exception expected but not thrown
    }
    catch (const std::invalid_argument& e)
    {
        ASSERT_STREQ("hub name cannot be empty", e.what());
    }
}