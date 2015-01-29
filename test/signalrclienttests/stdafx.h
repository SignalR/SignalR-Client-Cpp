// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#ifdef _WIN32
// prevents from defining min/max macros that conflict with std::min()/std::max() functions
#define NOMINMAX
#endif

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include "gtest\gtest.h"