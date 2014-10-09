// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include <cpprest\basic_types.h>
#include "http_sender.h"

using namespace signalr;

struct web_request_stub
{
    unsigned short m_status_code;
    utility::string_t m_response_body;
    utility::string_t m_method;
    utility::string_t m_user_agent_string;

    web_request_stub(unsigned short status_code, utility::string_t response_body) 
        : m_status_code(status_code), m_response_body(response_body)
    {}

    void set_method(const utility::string_t &method)
    {
        m_method = method;
    }

    void set_user_agent(const utility::string_t &user_agent_string)
    {
        m_user_agent_string = user_agent_string;
    }

    pplx::task<web_response> get_response()
    {
        return pplx::task_from_result<web_response>(
            web_response{ m_status_code, pplx::task_from_result<utility::string_t>(m_response_body) });
    }
};

TEST(http_sender_get_response, request_prepared_and_sent)
{
    utility::string_t response_body{ _XPLATSTR("response body") };
    web_request_stub request{ (unsigned short)200, response_body };

    ASSERT_EQ(response_body, http_sender::get<web_request_stub>(request).get());
}
