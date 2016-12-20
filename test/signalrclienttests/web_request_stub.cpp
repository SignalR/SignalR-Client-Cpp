// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "web_request_stub.h"

web_request_stub::web_request_stub(unsigned short status_code, const utility::string_t& reason_phrase, const utility::string_t& response_body)
    : web_request(web::uri(_XPLATSTR(""))), m_status_code(status_code), m_reason_phrase(reason_phrase), m_response_body(response_body)
{ }

void web_request_stub::set_method(const utility::string_t &method)
{
    m_method = method;
}

void web_request_stub::set_user_agent(const utility::string_t &user_agent_string)
{
    m_user_agent_string = user_agent_string;
}

pplx::task<web_response> web_request_stub::get_response(const signalr::signalr_client_config& signalr_client_config)
{
    m_headers = signalr_client_config.get_http_headers();
    on_get_response(*this);

    return pplx::task_from_result<web_response>(
        web_response{ m_status_code, m_reason_phrase, pplx::task_from_result<utility::string_t>(m_response_body) });
}