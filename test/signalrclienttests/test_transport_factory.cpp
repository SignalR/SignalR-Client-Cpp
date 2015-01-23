// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "test_transport_factory.h"
#include "websocket_transport.h"

test_transport_factory::test_transport_factory(const std::shared_ptr<websocket_client>& websocket_client)
    : m_websocket_client(websocket_client)
{ }

std::shared_ptr<transport> test_transport_factory::create_transport(transport_type transport_type, const logger& logger,
    const std::unordered_map<utility::string_t, utility::string_t>&,
    std::function<void(const utility::string_t&)> process_message_callback,
    std::function<void(const std::exception&)> error_callback)
{
    if (transport_type == transport_type::websockets)
    {
        return websocket_transport::create(m_websocket_client, logger, process_message_callback, error_callback);
    }

    throw std::exception("not supported");
}
