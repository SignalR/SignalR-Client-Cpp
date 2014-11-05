// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\ws_client.h>
#include "url_builder.h"
#include "signalrclient\transport.h"
#include "default_websocket_client.h"

namespace signalr
{
    class websocket_transport : public transport
    {
    public:

        websocket_transport(std::unique_ptr<websocket_client> websocket_client);

        websocket_transport(const websocket_transport&) = delete;

        websocket_transport& operator=(const websocket_transport&) = delete;

        pplx::task<void> connect(const web::uri &url) override;

        pplx::task<void> send(const utility::string_t &data) override;

        pplx::task<void> disconnect() override;

    private:
        std::unique_ptr<websocket_client> m_websocket_client;

        pplx::task_completion_event<void> m_connect_tce;

        void receive_loop();
    };
}