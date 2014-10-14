// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\basic_types.h>
#include "signalrclient\web_response.h"

using namespace signalr;

struct web_request_stub
{
    web::uri m_url;
    unsigned short m_status_code;
    utility::string_t m_reason_phrase;
    utility::string_t m_response_body;
    utility::string_t m_method;
    utility::string_t m_user_agent_string;

    web_request_stub(const web::uri &url) : m_url(url) 
    { }

    web_request_stub(unsigned short status_code, const utility::string_t &reason_phrase, const utility::string_t &response_body = _XPLATSTR("")) 
        : m_status_code(status_code), m_reason_phrase(reason_phrase), m_response_body(response_body)
    { }

    void set_method(utility::string_t method)
    {
        m_method = method;
    }

    void set_user_agent(utility::string_t user_agent_string)
    {
        m_user_agent_string = user_agent_string;
    }

    pplx::task<web_response> get_response()
    {
        return pplx::task_from_result<web_response>(
            web_response{ m_status_code, m_reason_phrase, pplx::task_from_result<utility::string_t>(m_response_body) });
    }
};
