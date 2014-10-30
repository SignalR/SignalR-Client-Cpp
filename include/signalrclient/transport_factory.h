// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <memory>
#include "signalrclient\transport_type.h"
#include "transport.h"

namespace signalr
{
    class transport_factory
    {
    public:
        std::unique_ptr<transport> virtual create_transport(transport_type transport_type);

        virtual ~transport_factory() {};
    };
}