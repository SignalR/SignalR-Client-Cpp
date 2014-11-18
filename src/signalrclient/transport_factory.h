// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <memory>
#include "signalrclient\transport_type.h"
#include "transport.h"

namespace signalr
{
    class connection_impl;

    class transport_factory
    {
    public:
        virtual std::shared_ptr<transport> create_transport(transport_type transport_type, std::shared_ptr<connection_impl> connection);

        virtual ~transport_factory();
    };
}