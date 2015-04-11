// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <stdexcept>
#include "cpprest/details/basic_types.h"
#include "cpprest/asyncrt_utils.h"

namespace signalr
{
    class web_exception : public std::runtime_error
    {
    public:
        explicit web_exception(const utility::string_t &what)
            : runtime_error(utility::conversions::to_utf8string(what))
        {}
    };
}