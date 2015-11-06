// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "test_web_request_factory.h"
#include "long_polling_transport.h"
#include "trace_log_writer.h"

using namespace signalr;

TEST(long_polling_transport_get_transport_type, get_transport_type_returns_long_polling)
{
    auto request_factory = std::make_shared<web_request_factory>();
    auto lp_transport = long_polling_transport::create(request_factory,
        logger(std::make_shared<trace_log_writer>(), trace_level::none),
        [](const utility::string_t&) {}, [](const std::exception&) {});

    ASSERT_EQ(transport_type::long_polling, lp_transport->get_transport_type());
}