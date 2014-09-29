// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

namespace signalr
{
    enum class trace_level
    {
        none,
        messages,
        events,
        state_changes,
        all = messages | events | state_changes
    };
}