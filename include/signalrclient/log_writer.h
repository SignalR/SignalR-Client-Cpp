// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "_exports.h"
#include <cpprest\basic_types.h>

namespace signalr
{
    class log_writer
    {
    public:
        // NOTE: the caller does not enforce thread safety of this call
        SIGNALRCLIENT_API virtual void write(const utility::string_t &entry) = 0;
    };
}