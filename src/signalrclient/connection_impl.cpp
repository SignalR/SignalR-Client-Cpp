// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "connection_impl.h"

namespace signalr
{
    pplx::task<void> connection_impl::start()
    {
        return pplx::task_from_result();
    }
}