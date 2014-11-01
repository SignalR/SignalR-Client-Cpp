// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "connection_impl.h"
#include "signalrclient\web_request_factory.h"
#include "signalrclient\transport_factory.h"

using namespace signalr;

TEST(connection_impl_connection_state, initial_connection_state_is_disconnected)
{
    web_request_factory request_factory;
    transport_factory transport_factory;

    connection_impl connection{ _XPLATSTR("url"), _XPLATSTR(""), request_factory, transport_factory };

    ASSERT_EQ(connection_state::disconnected, connection.get_connection_state());
}

TEST(connection_impl_start, cannot_start_non_disconnected_exception)
{
    web_request_factory request_factory;
    transport_factory transport_factory;

    connection_impl connection{ _XPLATSTR("url"), _XPLATSTR(""), request_factory, transport_factory };

    connection.start().wait();

    try
    {
        connection.start().wait();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const std::runtime_error& e)
    {
        ASSERT_EQ(
            _XPLATSTR("cannot start a connection that is not in the disconnected state"),
            utility::conversions::print_string(e.what()));
    }
}

TEST(connection_impl_start, connection_state_is_connecting_when_connection_is_being_started)
{
    web_request_factory request_factory;
    transport_factory transport_factory;

    connection_impl connection{ _XPLATSTR("url"), _XPLATSTR(""), request_factory, transport_factory };

    connection.start().wait();
    ASSERT_EQ(connection.get_connection_state(), connection_state::connecting);
}

