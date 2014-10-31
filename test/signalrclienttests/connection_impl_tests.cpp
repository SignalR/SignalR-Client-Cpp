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

    ASSERT_EQ(
        connection_state::disconnected, 
        connection_impl(_XPLATSTR("url"), _XPLATSTR(""), request_factory, transport_factory).get_connection_state());
}
