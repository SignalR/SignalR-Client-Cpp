// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "log_writer.h"

#ifdef _WIN32
#include <Windows.h>
#include <WinBase.h>
#endif

namespace signalr
{
    class trace_log_writer : public log_writer
    {
    public:
        void write(const utility::string_t &entry) override
        {
#ifdef _WIN32
            // OutputDebugString is thread safe
            OutputDebugString(entry.c_str());
#else
            // TODO: XPLAT - there is no data race for standard output streams in C++ 11 but the results
            // might be garbled when the method is called concurrently from multiple threads
#ifdef _UTF16_STRINGS
            std::wclog << entry;
#else
            std::clog << entry;
#endif  // _UTF16_STRINGS

#endif  // _MS_WINDOWS
        }
    };
}