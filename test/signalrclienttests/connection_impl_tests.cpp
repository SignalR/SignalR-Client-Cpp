// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "test_utils.h"
#include "connection_impl.h"
#include "signalrclient\trace_level.h"
#include "signalrclient\trace_log_writer.h"
#include "web_request_factory.h"
#include "transport_factory.h"
#include "memory_log_writer.h"

using namespace signalr;

TEST(connection_impl_connection_state, initial_connection_state_is_disconnected)
{
    auto connection = 
        connection_impl::create(_XPLATSTR("url"), _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>());

    ASSERT_EQ(connection_state::disconnected, connection->get_connection_state());
}

TEST(connection_impl_start, cannot_start_non_disconnected_exception)
{
    auto connection = 
        connection_impl::create(_XPLATSTR("url"), _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>());

    connection->start().wait();

    try
    {
        connection->start().wait();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::runtime_error& e)
    {
        ASSERT_EQ(
            _XPLATSTR("cannot start a connection that is not in the disconnected state"),
            utility::conversions::to_string_t(e.what()));
    }
}

TEST(connection_impl_start, connection_state_is_connecting_when_connection_is_being_started)
{
    auto connection = 
        connection_impl::create(_XPLATSTR("url"), _XPLATSTR(""), trace_level::none, std::make_shared<trace_log_writer>());

    connection->start().wait();
    ASSERT_EQ(connection->get_connection_state(), connection_state::connecting);
}

TEST(connection_impl_change_state, change_state_logs)
{
    std::shared_ptr<log_writer> writer(std::make_shared<memory_log_writer>());

    auto connection = 
        connection_impl::create(_XPLATSTR("url"), _XPLATSTR(""), trace_level::state_changes, writer);

    connection->start().wait();

    auto log_entries = std::dynamic_pointer_cast<memory_log_writer>(writer)->get_log_entries();
    ASSERT_FALSE(log_entries.empty());

    auto entry = remove_date_from_log_entry(log_entries[0]);
    ASSERT_EQ(_XPLATSTR("state changed: disconnected -> connecting\n"), entry);
}