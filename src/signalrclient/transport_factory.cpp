// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "transport_factory.h"
#include "websocket_transport.h"

namespace signalr
{
    std::shared_ptr<transport> transport_factory::create_transport(transport_type transport_type,
        const logger& logger, std::function<void(const utility::string_t&)> process_response_callback)
    {
        if (transport_type == transport_type::websockets)
        {
            return websocket_transport::create(
                std::make_shared<default_websocket_client>(), logger, process_response_callback);
        }

        throw std::exception("not implemented");
    }

    transport_factory::~transport_factory()
    { }
}