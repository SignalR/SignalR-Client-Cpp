// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "cpprest\details\basic_types.h"

namespace signalr
{
    struct negotiation_response
    {
        utility::string_t connection_id;
        utility::string_t connection_token;
        int disconnect_timeout; // in milliseconds
        int keep_alive_timeout; // in milliseconds

        utility::string_t protocol_version;
        bool try_websockets;
        int transport_connect_timeout; // in milliseconds
    };
}