// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "url_builder.h"

using namespace signalr;

TEST(url_builder_negotiate, url_correct_if_query_string_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/negotiate?clientProtocol=1.4")),
        url_builder::build_negotiate(web::uri{_XPLATSTR("http://fake/signalr/")}, _XPLATSTR("")));
}

TEST(url_builder_negotiate, url_correct_if_query_string_not_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/negotiate?clientProtocol=1.4&q1=1&q2=2")),
        url_builder::build_negotiate(web::uri{ _XPLATSTR("http://fake/signalr/") }, _XPLATSTR("q1=1&q2=2")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/negotiate?clientProtocol=1.4&q1=1&q2=2")),
        url_builder::build_negotiate(web::uri{ _XPLATSTR("http://fake/signalr/") }, _XPLATSTR("&q1=1&q2=2")));
}

TEST(url_builder_connect_longPolling, url_correct_if_query_string_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/connect?transport=longPolling&clientProtocol=1.4&connectionToken=connection%20token")),
        url_builder::build_connect(web::uri{ _XPLATSTR("http://fake/signalr/") },
            transport_type::long_polling, _XPLATSTR("connection token"), _XPLATSTR("")));
}

TEST(url_builder_connect_longPolling, url_correct_if_query_string_not_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/connect?transport=longPolling&clientProtocol=1.4&connectionToken=connection-token&q1=1&q2=2")),
        url_builder::build_connect(web::uri{ _XPLATSTR("http://fake/signalr/") },
            transport_type::long_polling, _XPLATSTR("connection-token"), _XPLATSTR("q1=1&q2=2")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/connect?transport=longPolling&clientProtocol=1.4&connectionToken=connection-token&q1=1&q2=2")),
        url_builder::build_connect(web::uri{ _XPLATSTR("http://fake/signalr/") },
            transport_type::long_polling, _XPLATSTR("connection-token"), _XPLATSTR("&q1=1&q2=2")));
}

TEST(url_builder_connect_webSockets, url_correct_if_query_string_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("ws://fake/signalr/connect?transport=webSockets&clientProtocol=1.4&connectionToken=connection%20token")),
        url_builder::build_connect(web::uri{ _XPLATSTR("http://fake/signalr/") },
            transport_type::websockets, _XPLATSTR("connection token"), _XPLATSTR("")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("wss://fake/signalr/connect?transport=webSockets&clientProtocol=1.4&connectionToken=connection%20token")),
        url_builder::build_connect(web::uri{ _XPLATSTR("https://fake/signalr/") },
        transport_type::websockets, _XPLATSTR("connection token"), _XPLATSTR("")));
}

TEST(url_builder_connect_webSockets, url_correct_if_query_string_not_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("ws://fake/signalr/connect?transport=webSockets&clientProtocol=1.4&connectionToken=connection-token&q1=1&q2=2")),
        url_builder::build_connect(web::uri{ _XPLATSTR("http://fake/signalr/") },
            transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR("q1=1&q2=2")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("ws://fake/signalr/connect?transport=webSockets&clientProtocol=1.4&connectionToken=connection-token&q1=1&q2=2")),
        url_builder::build_connect(web::uri{ _XPLATSTR("http://fake/signalr/") },
            transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR("&q1=1&q2=2")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("wss://fake/signalr/connect?transport=webSockets&clientProtocol=1.4&connectionToken=connection-token&q1=1&q2=2")),
        url_builder::build_connect(web::uri{ _XPLATSTR("https://fake/signalr/") },
        transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR("q1=1&q2=2")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("wss://fake/signalr/connect?transport=webSockets&clientProtocol=1.4&connectionToken=connection-token&q1=1&q2=2")),
        url_builder::build_connect(web::uri{ _XPLATSTR("https://fake/signalr/") },
        transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR("&q1=1&q2=2")));
}

TEST(url_builder_start, url_correct_if_query_string_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/start?transport=longPolling&clientProtocol=1.4&connectionToken=connection%20token")),
        url_builder::build_start(web::uri{ _XPLATSTR("http://fake/signalr/") },
        transport_type::long_polling, _XPLATSTR("connection token"), _XPLATSTR("")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/start?transport=webSockets&clientProtocol=1.4&connectionToken=connection%20token")),
        url_builder::build_start(web::uri{ _XPLATSTR("http://fake/signalr/") },
        transport_type::websockets, _XPLATSTR("connection token"), _XPLATSTR("")));
}

TEST(url_builder_start, url_correct_if_query_string_not_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/start?transport=longPolling&clientProtocol=1.4&connectionToken=connection-token&q1=1&q2=2")),
        url_builder::build_start(web::uri{ _XPLATSTR("http://fake/signalr/") },
        transport_type::long_polling, _XPLATSTR("connection-token"), _XPLATSTR("q1=1&q2=2")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/start?transport=webSockets&clientProtocol=1.4&connectionToken=connection-token&q1=1&q2=2")),
        url_builder::build_start(web::uri{ _XPLATSTR("http://fake/signalr/") },
        transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR("&q1=1&q2=2")));
}

TEST(url_builder_abort, url_correct_if_query_string_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/abort?transport=longPolling&clientProtocol=1.4&connectionToken=connection%20token")),
        url_builder::build_abort(web::uri{ _XPLATSTR("http://fake/signalr/") },
        transport_type::long_polling, _XPLATSTR("connection token"), _XPLATSTR("")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/abort?transport=webSockets&clientProtocol=1.4&connectionToken=connection%20token")),
        url_builder::build_abort(web::uri{ _XPLATSTR("http://fake/signalr/") },
        transport_type::websockets, _XPLATSTR("connection token"), _XPLATSTR("")));
}

TEST(url_builder_abort, url_correct_if_query_string_not_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/abort?transport=longPolling&clientProtocol=1.4&connectionToken=connection-token&q1=1&q2=2")),
        url_builder::build_abort(web::uri{ _XPLATSTR("http://fake/signalr/") },
        transport_type::long_polling, _XPLATSTR("connection-token"), _XPLATSTR("q1=1&q2=2")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/signalr/abort?transport=webSockets&clientProtocol=1.4&connectionToken=connection-token&q1=1&q2=2")),
        url_builder::build_abort(web::uri{ _XPLATSTR("http://fake/signalr/") },
        transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR("&q1=1&q2=2")));
}