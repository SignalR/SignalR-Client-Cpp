// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "default_websocket_client.h"

namespace signalr
{
    pplx::task<void> default_websocket_client::connect(const web::uri &url)
    {
        return m_underlying_client.connect(url);
    }

    pplx::task<void> default_websocket_client::send(const utility::string_t &message)
    {
        web_sockets::client::websocket_outgoing_message msg;
        msg.set_utf8_message(utility::conversions::to_utf8string(message));
        return m_underlying_client.send(msg);
    }

    pplx::task<std::string> default_websocket_client::receive()
    {
        return m_underlying_client.receive()
            .then([](web_sockets::client::websocket_incoming_message msg)
            {
                return msg.extract_string();
            });
    }

    pplx::task<void> default_websocket_client::close()
    {
        return m_underlying_client.close();
    }
}