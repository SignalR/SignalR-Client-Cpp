// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "transport_factory.h"
#include "websocket_transport.h"
#include "connection_impl.h"

namespace signalr
{
    std::unique_ptr<transport> transport_factory::create_transport(transport_type transport_type, std::shared_ptr<connection_impl> connection)
    {
        if (transport_type == transport_type::websockets)
        {
            return std::make_unique<websocket_transport>(std::make_unique<default_websocket_client>(), connection);
        }

        throw std::exception("not implemented");
    }

    transport_factory::~transport_factory()
    { }
}