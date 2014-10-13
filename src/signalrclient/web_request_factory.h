// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\base_uri.h>

namespace signalr
{
    template<typename T>
    class web_request_factory
    {
    public:
        virtual T create_web_request(const web::uri &url) const
        {
            return T(url);
        }
    };
}