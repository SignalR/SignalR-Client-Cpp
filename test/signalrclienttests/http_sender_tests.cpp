// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include <cpprest\basic_types.h>
#include <cpprest\asyncrt_utils.h>
#include "signalrclient\web_exception.h"
#include "http_sender.h"
#include "web_request_stub.h"

TEST(http_sender_get_response, request_sent_using_get_method)
{
    utility::string_t response_body{ _XPLATSTR("response body") };
    web_request_stub request{ (unsigned short)200, _XPLATSTR("OK"), response_body };

    ASSERT_EQ(response_body, http_sender::get(request).get());
    ASSERT_EQ(_XPLATSTR("GET"), request.m_method);
}

TEST(http_sender_get_response, exception_thrown_if_status_code_not_200)
{
    utility::string_t response_phrase(_XPLATSTR("Custom Not Found"));
    web_request_stub request{ (unsigned short)404, response_phrase };

    try
    {
        http_sender::get(request).get();
        ASSERT_TRUE(false); // exception not thrown
    }
    catch (const web_exception &e)
    {
        ASSERT_EQ(
            _XPLATSTR("web exception - 404 ") + response_phrase,
            utility::conversions::to_string_t(e.what()));
    }
}

TEST(http_sender_get_response, user_agent_set)
{
    utility::string_t response_body{ _XPLATSTR("response body") };
    web_request_stub request{ (unsigned short)200, response_body };

    http_sender::get(request).get();

    ASSERT_EQ(_XPLATSTR("SignalR.Client.Cpp/3.0.0"), request.m_user_agent_string);
}