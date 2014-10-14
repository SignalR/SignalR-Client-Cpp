// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include <cpprest\http_client.h>
#include "signalrclient\web_request.h"

namespace signalr
{
    void web_request::set_method(const utility::string_t &method)
    {
        m_method = method;
    }

    void web_request::set_user_agent(const utility::string_t &user_agent_string)
    {
        m_user_agent_string = user_agent_string;
    }

    pplx::task<web_response> web_request::get_response() const
    {
        web::http::client::http_client client(m_url);
        web::http::http_request request(m_method);
        request.headers()[_XPLATSTR("User-Agent")] = m_user_agent_string;

        return client.request(request)
            .then([](web::http::http_response response)
            {
                return web_response
                { 
                    response.status_code(), 
                    response.reason_phrase(), 
                    response.extract_string() 
                };
            });
    }
}