// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "test_transport_factory.h"
#include "websocket_transport.h"

test_transport_factory::test_transport_factory(std::shared_ptr<websocket_client> websocket_client)
    : m_websocket_client(websocket_client)
{ }

std::shared_ptr<transport> test_transport_factory::create_transport(transport_type transport_type,
    logger logger, std::function<void(utility::string_t)> process_message)
{
    if (transport_type == transport_type::websockets)
    {
        return websocket_transport::create(m_websocket_client, logger, process_message);
    }

    throw std::exception("not supported");
}
