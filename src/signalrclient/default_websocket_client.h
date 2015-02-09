// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <unordered_map>
#include "cpprest\ws_client.h"
#include "websocket_client.h"

namespace signalr
{
    class default_websocket_client : public websocket_client
    {
    public:
        explicit default_websocket_client(const std::unordered_map<utility::string_t, utility::string_t>& headers);

        pplx::task<void> connect(const web::uri &url) override;

        pplx::task<void> send(const utility::string_t &message) override;

        pplx::task<std::string> receive() override;

        pplx::task<void> close() override;

    private:
        web::websockets::client::websocket_client m_underlying_client;
    };
}