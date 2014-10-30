// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "signalrclient\transport_factory.h"
#include "websocket_transport.h"

namespace signalr
{
    std::unique_ptr<transport> transport_factory::create_transport(transport_type transport_type)
    {
        if (transport_type == transport_type::websockets)
        {
            return std::make_unique<websocket_transport<>>();
        }

        throw std::exception("not implemented");
    }

}