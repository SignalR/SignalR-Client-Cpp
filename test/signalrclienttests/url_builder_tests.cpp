#include "stdafx.h"
#include "url_builder.h"

using namespace web;
using namespace signalr;

TEST(url_builder_negotiate, url_correct_if_query_string_empty)
{
    ASSERT_EQ(
        uri(_XPLATSTR("negotiate?clientProtocol=1.4")),
        signalr::url_builder::build_negotiate(_XPLATSTR("")));
}

TEST(url_builder, url_correct_if_query_string_not_empty)
{
    ASSERT_EQ(
        uri(_XPLATSTR("negotiate?clientProtocol=1.4&q1=1&q2=2")),
        signalr::url_builder::build_negotiate(_XPLATSTR("q1=1&q2=2")));

    ASSERT_EQ(
        uri(_XPLATSTR("negotiate?clientProtocol=1.4&q1=1&q2=2")),
        signalr::url_builder::build_negotiate(_XPLATSTR("&q1=1&q2=2")));
}