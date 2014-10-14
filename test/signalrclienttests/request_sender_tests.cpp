// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include <cpprest\basic_types.h>
#include "request_sender.h"
#include "web_request_stub.h"
#include "test_web_request_factory.h"

using namespace signalr;

// this class should not be visible outside this file
template<typename T>
class request_factory_fake : public web_request_factory<T>
{
private:
    T m_web_request;
    mutable web::uri m_last_url;

public:

    request_factory_fake(const T &web_request)
        : m_web_request(web_request)
    { }

    T create_web_request(const web::uri &url) const
    {
        m_last_url = url;
        return m_web_request;
    }

    web::uri get_last_url() const
    {
        return m_last_url;
    }
};

TEST(request_sender_negotiate, request_created_with_correct_url)
{
    utility::string_t response_body(
        _XPLATSTR("{\"Url\":\"/signalr\", \"ConnectionToken\" : \"A==\", \"ConnectionId\" : \"f7707523-307d-4cba-9abf-3eef701241e8\", ")
        _XPLATSTR("\"KeepAliveTimeout\" : 20.0, \"DisconnectTimeout\" : 30.0, \"ConnectionTimeout\" : 110.0, \"TryWebSockets\" : true, ")
        _XPLATSTR("\"ProtocolVersion\" : \"1.4\", \"TransportConnectTimeout\" : 5.0, \"LongPollDelay\" : 0.0}"));

    request_factory_fake<web_request_stub> request_factory{ web_request_stub{ (unsigned short)200, _XPLATSTR("OK"), response_body } };

    request_sender::negotiate<web_request_stub>(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") }, _XPLATSTR("")).get();

    ASSERT_EQ(web::uri(_XPLATSTR("http://fake/signalr/negotiate?clientProtocol=1.4")), request_factory.get_last_url());
}

TEST(request_sender_negotiate, negotiation_request_sent_and_response_serialized)
{
    utility::string_t response_body(
        _XPLATSTR("{\"Url\":\"/signalr\", \"ConnectionToken\" : \"A==\", \"ConnectionId\" : \"f7707523-307d-4cba-9abf-3eef701241e8\", ")
        _XPLATSTR("\"KeepAliveTimeout\" : 20.0, \"DisconnectTimeout\" : 30.0, \"ConnectionTimeout\" : 110.0, \"TryWebSockets\" : true, ")
        _XPLATSTR("\"ProtocolVersion\" : \"1.4\", \"TransportConnectTimeout\" : 5.5, \"LongPollDelay\" : 0.0}"));

    test_web_request_factory<web_request_stub> request_factory{ web_request_stub{ (unsigned short)200, _XPLATSTR("OK"), response_body } };

    auto response = request_sender::negotiate<web_request_stub>(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") }, _XPLATSTR("")).get();

    ASSERT_EQ(_XPLATSTR("f7707523-307d-4cba-9abf-3eef701241e8"), response.connection_id);
    ASSERT_EQ(_XPLATSTR("A=="), response.connection_token);
    ASSERT_EQ(30000, response.disconnect_timeout);
    ASSERT_EQ(20000, response.keep_alive_timeout);
    ASSERT_EQ(_XPLATSTR("1.4"), response.protocol_version);
    ASSERT_TRUE(response.try_websockets);
    ASSERT_EQ(5500, response.transport_connect_timeout);
}