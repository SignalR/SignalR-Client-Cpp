// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "url_builder.h"

using namespace signalr;

TEST(url_builder_negotiate, url_correct_if_query_string_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/sigalr/negotiate?clientProtocol=1.4")),
        url_builder::build_negotiate(web::uri{_XPLATSTR("http://fake/sigalr/")}, _XPLATSTR("")));
}

TEST(url_builder, url_correct_if_query_string_not_empty)
{
    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/sigalr/negotiate?clientProtocol=1.4&q1=1&q2=2")),
        url_builder::build_negotiate(web::uri{ _XPLATSTR("http://fake/sigalr/") }, _XPLATSTR("q1=1&q2=2")));

    ASSERT_EQ(
        web::uri(_XPLATSTR("http://fake/sigalr/negotiate?clientProtocol=1.4&q1=1&q2=2")),
        url_builder::build_negotiate(web::uri{ _XPLATSTR("http://fake/sigalr/") }, _XPLATSTR("&q1=1&q2=2")));
}