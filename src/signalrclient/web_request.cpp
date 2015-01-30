// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "cpprest\http_client.h"
#include "web_request.h"

namespace signalr
{
    web_request::web_request(const web::uri &url)
        : m_url(url)
    { }

    void web_request::set_method(const utility::string_t &method)
    {
        m_request.set_method(method);
    }

    void web_request::set_user_agent(const utility::string_t &user_agent_string)
    {
        m_request.headers()[_XPLATSTR("User-Agent")] = user_agent_string;
    }

    void web_request::set_headers(const std::unordered_map<utility::string_t, utility::string_t>& headers)
    {
        for (auto &header : headers)
        {
            m_request.headers()[header.first] = header.second;
        }
    }

    pplx::task<web_response> web_request::get_response()
    {
        web::http::client::http_client client(m_url);

        return client.request(m_request)
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

	web_request::~web_request() = default;
}