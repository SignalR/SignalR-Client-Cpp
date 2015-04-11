// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <memory>
#include <unordered_map>
#include "signalrclient/transport_type.h"
#include "transport.h"

namespace signalr
{
    class transport_factory
    {
    public:
        virtual std::shared_ptr<transport> create_transport(transport_type transport_type, const logger& logger,
            const std::unordered_map<utility::string_t, utility::string_t>& headers,
            std::function<void(const utility::string_t&)> process_response_callback,
            std::function<void(const std::exception&)> error_callback);

        virtual ~transport_factory();
    };
}