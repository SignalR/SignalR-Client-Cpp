// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "transport_factory.h"
#include "websocket_transport.h"

namespace signalr
{
    std::shared_ptr<transport> transport_factory::create_transport(transport_type transport_type, const logger& logger,
        const std::unordered_map<utility::string_t, utility::string_t>& headers,
        std::function<void(const utility::string_t&)> process_response_callback,
        std::function<void(const std::exception&)> error_callback)
    {
        if (transport_type == signalr::transport_type::websockets)
        {
            return websocket_transport::create([headers](){ return std::make_shared<default_websocket_client>(headers); },
                logger, process_response_callback, error_callback);
        }

        throw std::runtime_error("not implemented");
    }

    transport_factory::~transport_factory()
    { }
}