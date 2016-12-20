// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <memory>
#include "_exports.h"

namespace web
{
    class web_proxy;
    class credentials;
    namespace http
    {
        namespace client
        {
            class http_client_config;
        }
    }
    namespace websockets
    {
        namespace client
        {
            class websocket_client_config;
        }
    }
}


namespace signalr
{
    //If you need access to the underlying configuration objects, you need to include
    //cpprest/http_client.h and/or cpprest/ws_client.h
    class signalr_client_config
    {
    public:
        SIGNALRCLIENT_API ~signalr_client_config();
        SIGNALRCLIENT_API signalr_client_config();
        //VS2013 doesn't synthesize move operations, so we do them manually
        SIGNALRCLIENT_API signalr_client_config(signalr_client_config&& other);
        SIGNALRCLIENT_API signalr_client_config& operator=(signalr_client_config&& other);
        SIGNALRCLIENT_API signalr_client_config(const signalr_client_config& other);
        SIGNALRCLIENT_API signalr_client_config& operator=(const signalr_client_config& other);

        SIGNALRCLIENT_API void set_proxy(const web::web_proxy &proxy);
        // Please note that setting credentials does not work in all cases.
        // For example, Basic Authentication fails under Win32.
        // As a workaround, you can set the required headers directly by
        // using connection::set_headers or hub_connection::set_headers
        SIGNALRCLIENT_API void set_credentials(const web::credentials &cred);

        SIGNALRCLIENT_API web::http::client::http_client_config& http_client_config();
        SIGNALRCLIENT_API web::websockets::client::websocket_client_config& websocket_client_config();

    private:
        std::unique_ptr<web::http::client::http_client_config> m_http_client_config;
        std::unique_ptr<web::websockets::client::websocket_client_config> m_websocket_client_config;
    };
}