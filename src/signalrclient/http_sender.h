// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <ppl.h>
#include <cpprest\basic_types.h>

#include "signalrclient\web_request.h"

namespace pplx = concurrency;

namespace signalr
{
    namespace http_sender
    {
        pplx::task<utility::string_t> get(web_request &request);
    }
}