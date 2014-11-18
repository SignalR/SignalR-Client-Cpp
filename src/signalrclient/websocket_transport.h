// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\ws_client.h>
#include "url_builder.h"
#include "transport.h"
#include "logger.h"
#include "default_websocket_client.h"
#include "connection_impl.h"

namespace signalr
{
    class websocket_transport : public transport, public std::enable_shared_from_this<websocket_transport>
    {
    public:
        static std::shared_ptr<transport> create(std::shared_ptr<websocket_client> websocket_client,
            std::shared_ptr<connection_impl> connection, std::function<void(const utility::string_t&)> process_response_callback);

        ~websocket_transport();

        websocket_transport(const websocket_transport&) = delete;

        websocket_transport& operator=(const websocket_transport&) = delete;

        pplx::task<void> connect(const web::uri &url) override;

        pplx::task<void> send(const utility::string_t &data) override;

        pplx::task<void> disconnect() override;

    private:
        websocket_transport(std::shared_ptr<websocket_client> websocket_client,
            std::shared_ptr<connection_impl> connection, std::function<void(const utility::string_t &)> process_response_callback);

        std::shared_ptr<websocket_client> m_websocket_client;

        pplx::cancellation_token_source m_receive_loop_cts;

        void receive_loop(pplx::cancellation_token_source cts);
    };
}