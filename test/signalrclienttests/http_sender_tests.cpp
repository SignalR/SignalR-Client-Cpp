// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "cpprest/details/basic_types.h"
#include "cpprest/asyncrt_utils.h"
#include "signalrclient/web_exception.h"
#include "http_sender.h"
#include "web_request_stub.h"
#include "test_web_request_factory.h"

namespace { typedef std::unordered_map<utility::string_t, utility::string_t> headers_t; }

TEST(http_sender_get_response, request_sent_using_get_method)
{
    utility::string_t response_body{ _XPLATSTR("response body") };

    auto web_request_factory = std::make_unique<test_web_request_factory>([response_body](const web::uri &) -> std::unique_ptr<web_request>
    {
        auto request = new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body);
        request->on_get_response = [](web_request_stub& request) { ASSERT_EQ(_XPLATSTR("GET"), request.m_method); };

        return std::unique_ptr<web_request>(request);
    });

    ASSERT_EQ(response_body, http_sender::get(*web_request_factory, _XPLATSTR("url"), headers_t{}).get());
}

TEST(http_sender_get_response, exception_thrown_if_status_code_not_200)
{
    utility::string_t response_phrase(_XPLATSTR("Custom Not Found"));

    auto web_request_factory = std::make_unique<test_web_request_factory>([response_phrase](const web::uri &) -> std::unique_ptr<web_request>
    {
        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)404, response_phrase));
    });

    try
    {
        http_sender::get(*web_request_factory, _XPLATSTR("url"), headers_t{}).get();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const web_exception &e)
    {
        ASSERT_EQ(
            _XPLATSTR("web exception - 404 ") + response_phrase,
            utility::conversions::to_string_t(e.what()));
        ASSERT_EQ(404, e.status_code());
    }
}

TEST(http_sender_get_response, user_agent_set)
{
    utility::string_t response_body{ _XPLATSTR("response body") };

    auto web_request_factory = std::make_unique<test_web_request_factory>([response_body](const web::uri &) -> std::unique_ptr<web_request>
    {
        auto request = new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body);
        request->on_get_response = [](web_request_stub& request)
        {
            ASSERT_EQ(_XPLATSTR("SignalR.Client.Cpp/1.0.0-beta1"), request.m_user_agent_string);
        };

        return std::unique_ptr<web_request>(request);
    });

    ASSERT_EQ(response_body, http_sender::get(*web_request_factory, _XPLATSTR("url"), headers_t{}).get());
}

TEST(http_sender_get_response, headers_set)
{
    utility::string_t response_body{ _XPLATSTR("response body") };

    auto web_request_factory = std::make_unique<test_web_request_factory>([response_body](const web::uri &) -> std::unique_ptr<web_request>
    {
        auto request = new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body);
        request->on_get_response = [](web_request_stub& request)
        {
            ASSERT_EQ(1, request.m_headers.size());
            ASSERT_EQ(_XPLATSTR("123"), request.m_headers[_XPLATSTR("abc")]);
        };

        return std::unique_ptr<web_request>(request);
    });

    headers_t headers;
    headers[_XPLATSTR("abc")] = _XPLATSTR("123");

    // ensures that web_request.get_response() was invoked
    ASSERT_EQ(response_body, http_sender::get(*web_request_factory, _XPLATSTR("url"), headers).get());
}