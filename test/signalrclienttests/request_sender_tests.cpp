// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "cpprest/details/basic_types.h"
#include "signalrclient/web_exception.h"
#include "request_sender.h"
#include "web_request_stub.h"
#include "test_web_request_factory.h"
#include "signalrclient/signalr_exception.h"

using namespace signalr;

TEST(request_sender_negotiate, request_created_with_correct_url)
{
    web::uri requested_url;
    auto request_factory = test_web_request_factory([&requested_url](const web::uri &url) -> std::unique_ptr<web_request>
    {
        utility::string_t response_body(
            _XPLATSTR("{\"Url\":\"/signalr\", \"ConnectionToken\" : \"A==\", \"ConnectionId\" : \"f7707523-307d-4cba-9abf-3eef701241e8\", ")
            _XPLATSTR("\"KeepAliveTimeout\" : 20.0, \"DisconnectTimeout\" : 30.0, \"ConnectionTimeout\" : 110.0, \"TryWebSockets\" : true, ")
            _XPLATSTR("\"ProtocolVersion\" : \"1.4\", \"TransportConnectTimeout\" : 5.0, \"LongPollDelay\" : 0.0}"));

        requested_url = url;
        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });

    request_sender::negotiate(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") }, _XPLATSTR("data"), _XPLATSTR("")).get();

    ASSERT_EQ(web::uri(_XPLATSTR("http://fake/signalr/negotiate?clientProtocol=1.4&connectionData=data")), requested_url);
}

TEST(request_sender_negotiate, negotiation_request_sent_and_response_serialized)
{
    auto request_factory = test_web_request_factory([](const web::uri&) -> std::unique_ptr<web_request>
    {
        utility::string_t response_body(
            _XPLATSTR("{\"Url\":\"/signalr\", \"ConnectionToken\" : \"A==\", \"ConnectionId\" : \"f7707523-307d-4cba-9abf-3eef701241e8\", ")
            _XPLATSTR("\"KeepAliveTimeout\" : 20.0, \"DisconnectTimeout\" : 30.0, \"ConnectionTimeout\" : 110.0, \"TryWebSockets\" : true, ")
            _XPLATSTR("\"ProtocolVersion\" : \"1.4\", \"TransportConnectTimeout\" : 5.5, \"LongPollDelay\" : 0.0}"));

        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });

    auto response = request_sender::negotiate(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") },
        _XPLATSTR(""), _XPLATSTR("")).get();

    ASSERT_EQ(_XPLATSTR("f7707523-307d-4cba-9abf-3eef701241e8"), response.connection_id);
    ASSERT_EQ(_XPLATSTR("A=="), response.connection_token);
    ASSERT_EQ(30000, response.disconnect_timeout);
    ASSERT_EQ(20000, response.keep_alive_timeout);
    ASSERT_EQ(_XPLATSTR("1.4"), response.protocol_version);
    ASSERT_TRUE(response.try_websockets);
    ASSERT_EQ(5500, response.transport_connect_timeout);
}

TEST(request_sender_negotiate, negotiate_can_handle_null_keep_alive_timeout)
{
    utility::string_t test_values[] = { _XPLATSTR(""), _XPLATSTR("\"KeepAliveTimeout\" : null, ") };

    for (const auto& keep_alive_timeout : test_values)
    {
        auto request_factory = test_web_request_factory([keep_alive_timeout](const web::uri&) -> std::unique_ptr < web_request >
        {
            utility::string_t response_body(
                utility::string_t(_XPLATSTR("{\"Url\":\"/signalr\", \"ConnectionToken\" : \"A==\", \"ConnectionId\" : \"f7707523-307d-4cba-9abf-3eef701241e8\", "))
                .append(keep_alive_timeout)
                .append(_XPLATSTR("\"KeepAliveTimeout\" : null, \"DisconnectTimeout\" : 30.0, \"ConnectionTimeout\" : 110.0, \"TryWebSockets\" : true, ")
                _XPLATSTR("\"ProtocolVersion\" : \"1.4\", \"TransportConnectTimeout\" : 5.5, \"LongPollDelay\" : 0.0}")));

            return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
        });

        auto response = request_sender::negotiate(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") },
            _XPLATSTR(""), _XPLATSTR("")).get();

        ASSERT_EQ(-1, response.keep_alive_timeout);
    }
}

TEST(request_sender_start, start_does_not_throw_if_transport_started_successfully)
{
    auto request_factory = test_web_request_factory([](const web::uri&)
    {
        utility::string_t response_body(_XPLATSTR("{\"Response\":\"started\" }"));

        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });

    ASSERT_NO_THROW(request_sender::start(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") },
        transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR(""), _XPLATSTR("")).get());
}

TEST(request_sender_start, request_sent_with_correct_url)
{
    utility::string_t actual_url;
    auto request_factory = test_web_request_factory([&actual_url](const web::uri& url)
    {
        actual_url = url.to_string();
        utility::string_t response_body(_XPLATSTR("{\"Response\":\"started\" }"));
        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });

    request_sender::start(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") },
        transport_type::websockets, _XPLATSTR("abc"), _XPLATSTR("data"), _XPLATSTR("")).get();

    ASSERT_EQ(_XPLATSTR("http://fake/signalr/start?transport=webSockets&clientProtocol=1.4&connectionToken=abc&connectionData=data"), actual_url);
}

TEST(request_sender_start, start_request_returns_false_if_response_is_not_started_literal)
{
    auto request_factory = test_web_request_factory([](const web::uri&)
    {
        utility::string_t response_body(_XPLATSTR("{\"Response\":\"42\" }"));

        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });

    try
    {
        request_sender::start(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") },
            transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR(""), _XPLATSTR("")).get();

        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const signalr_exception& e)
    {
        ASSERT_STREQ("start request failed due to unexpected response from the server: {\"Response\":\"42\" }", e.what());
    }
}

TEST(request_sender_start, start_request_returns_false_if_response_missing)
{
    auto request_factory = test_web_request_factory([](const web::uri&)
    {
        utility::string_t response_body(_XPLATSTR("{}"));

        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });

    try
    {
        request_sender::start(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") },
            transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR(""), _XPLATSTR("")).get();

        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const signalr_exception& e)
    {
        ASSERT_STREQ("start request failed due to unexpected response from the server: {}", e.what());
    }
}

TEST(request_sender_start, start_propagates_exceptions)
{
    auto request_factory = test_web_request_factory([](const web::uri&)
    {
        return std::unique_ptr<web_request>(
            new web_request_stub((unsigned short)503, _XPLATSTR("Server unavailable"), _XPLATSTR("")));
    });

    try
    {
        request_sender::start(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") },
            transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR(""), _XPLATSTR("")).get();

        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const web_exception& e)
    {
        ASSERT_EQ(_XPLATSTR("web exception - 503 Server unavailable"), utility::conversions::to_string_t(e.what()));
        ASSERT_EQ(503, e.status_code());
    }
}

TEST(request_sender_abort, request_sent_with_correct_url)
{
    auto request_factory = test_web_request_factory([](const web::uri& url)
    {
        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), url.to_string()));
    });

    auto actual_url = request_sender::abort(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") },
        transport_type::websockets, _XPLATSTR("abc"), _XPLATSTR("data"), _XPLATSTR("")).get();

    ASSERT_EQ(_XPLATSTR("http://fake/signalr/abort?transport=webSockets&clientProtocol=1.4&connectionToken=abc&connectionData=data"), actual_url);
}

TEST(request_sender_abort, abort_propagates_exceptions)
{
    auto request_factory = test_web_request_factory([](const web::uri&)
    {
        return std::unique_ptr<web_request>(
            new web_request_stub((unsigned short)503, _XPLATSTR("Server unavailable"), _XPLATSTR("")));
    });

    try
    {
        request_sender::abort(request_factory, web::uri{ _XPLATSTR("http://fake/signalr") },
            transport_type::websockets, _XPLATSTR("connection-token"), _XPLATSTR(""), _XPLATSTR("")).get();

        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const web_exception& e)
    {
        ASSERT_EQ(_XPLATSTR("web exception - 503 Server unavailable"), utility::conversions::to_string_t(e.what()));
        ASSERT_EQ(503, e.status_code());
    }
}