// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once;

#include <cpprest/ws_client.h>

namespace signalr
{
    class transport
    {
    public:
        virtual pplx::task<void> connect(const web::uri &url) = 0;

        virtual pplx::task<void> send(const utility::string_t &data) = 0;

        virtual pplx::task<void> disconnect() = 0;

        virtual ~transport() {};
    };
}