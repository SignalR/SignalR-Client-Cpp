// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "signalrclient/signalr_client_config.h"
#include "cpprest/http_client.h"
#include "cpprest/ws_client.h"

namespace signalr
{
    signalr_client_config::~signalr_client_config() {}

    signalr_client_config::signalr_client_config()
        : m_http_client_config{ std::make_unique<web::http::client::http_client_config>() }
        , m_websocket_client_config{ std::make_unique<web::websockets::client::websocket_client_config>() }
    {}

    signalr_client_config::signalr_client_config(signalr_client_config&& other)
    {
        m_http_client_config = std::move(other.m_http_client_config);
        m_websocket_client_config = std::move(other.m_websocket_client_config);
    }

    signalr_client_config& signalr_client_config::operator = (signalr_client_config&& other)
    {
        if (this != &other)
        {
            m_http_client_config = std::move(other.m_http_client_config);
            m_websocket_client_config = std::move(other.m_websocket_client_config);
        }
        return *this;
    }

    signalr_client_config::signalr_client_config(const signalr_client_config& other)
        : m_http_client_config{ std::make_unique<web::http::client::http_client_config>(*other.m_http_client_config) }
        , m_websocket_client_config{ std::make_unique<web::websockets::client::websocket_client_config>(*other.m_websocket_client_config)}
    {
    }

    signalr_client_config& signalr_client_config::operator = (const signalr_client_config& other)
    {
        m_http_client_config = std::make_unique<web::http::client::http_client_config>(*other.m_http_client_config);
        m_websocket_client_config = std::make_unique<web::websockets::client::websocket_client_config>(*other.m_websocket_client_config);
        return *this;
    }


    void signalr_client_config::set_proxy(const web::web_proxy &proxy)
    {
        m_http_client_config->set_proxy(proxy);
        m_websocket_client_config->set_proxy(proxy);
    }

    void signalr_client_config::set_credentials(const web::credentials &cred)
    {
        m_http_client_config->set_credentials(cred);
        m_websocket_client_config->set_credentials(cred);
    }

    web::http::client::http_client_config& signalr_client_config::http_client_config()
    {
        return *m_http_client_config;
    }

    web::websockets::client::websocket_client_config& signalr_client_config::websocket_client_config()
    {
        return *m_websocket_client_config;
    }
}