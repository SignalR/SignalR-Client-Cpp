// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\basic_types.h>
#include "signalrclient\web_response.h"
#include "signalrclient\web_request.h"

using namespace signalr;

struct web_request_stub : public web_request
{
    unsigned short m_status_code;
    utility::string_t m_reason_phrase;
    utility::string_t m_response_body;
    utility::string_t m_method;
    utility::string_t m_user_agent_string;

    web_request_stub(unsigned short status_code, const utility::string_t& reason_phrase, const utility::string_t& response_body = _XPLATSTR(""))
        : web_request(web::uri(_XPLATSTR(""))), m_status_code(status_code), m_reason_phrase(reason_phrase), m_response_body(response_body)
    { }

    virtual void set_method(const utility::string_t &method) override
    {
        m_method = method;
    }

    virtual void set_user_agent(const utility::string_t &user_agent_string) override
    {
        m_user_agent_string = user_agent_string;
    }

    virtual pplx::task<web_response> get_response() override
    {
        return pplx::task_from_result<web_response>(
            web_response{ m_status_code, m_reason_phrase, pplx::task_from_result<utility::string_t>(m_response_body) });
    }
};
