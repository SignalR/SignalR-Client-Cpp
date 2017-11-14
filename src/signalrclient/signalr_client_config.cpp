// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "signalrclient/signalr_client_config.h"
#include "cpprest/http_client.h"
#include "cpprest/ws_client.h"
#include "constants.h"

namespace signalr
{
    signalr_client_config::signalr_client_config()
    {
        m_user_agent_string = USER_AGENT;
    }

    void signalr_client_config::set_proxy(const web::web_proxy &proxy)
    {
        m_http_client_config.set_proxy(proxy);
        m_websocket_client_config.set_proxy(proxy);
    }

    void signalr_client_config::set_credentials(const web::credentials &credentials)
    {
        m_http_client_config.set_credentials(credentials);
        m_websocket_client_config.set_credentials(credentials);
    }

    web::http::client::http_client_config signalr_client_config::get_http_client_config() const
    {
        return m_http_client_config;
    }

    void signalr_client_config::set_http_client_config(const web::http::client::http_client_config& http_client_config)
    {
        m_http_client_config = http_client_config;
    }

    web::websockets::client::websocket_client_config signalr_client_config::get_websocket_client_config() const
    {
        return m_websocket_client_config;
    }

    void signalr_client_config::set_websocket_client_config(const web::websockets::client::websocket_client_config& websocket_client_config)
    {
        m_websocket_client_config = websocket_client_config;
    }

    web::http::http_headers signalr_client_config::get_http_headers() const
    {
        return m_http_headers;
    }

    void signalr_client_config::set_http_headers(const web::http::http_headers& http_headers)
    {
        m_http_headers = http_headers;
    }

    const utility::string_t& signalr_client_config::get_user_agent() const
    {
        return m_user_agent_string;
    }

    void signalr_client_config::set_user_agent(const utility::string_t& user_agent_string)
    {
        m_user_agent_string = user_agent_string;
    }
}