// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <ppl.h>
#include <cpprest\base_uri.h>

namespace pplx = concurrency;

namespace signalr
{
    class websocket_client
    {
    public:
        virtual pplx::task<void> connect(const web::uri &url) = 0;

        virtual pplx::task<void> send(const utility::string_t &message) = 0;

        virtual pplx::task<std::string> receive() = 0;

        virtual pplx::task<void> close() = 0;

        virtual ~websocket_client() {};
    };
}