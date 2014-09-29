// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#ifdef SIGNALRCLIENT_EXPORTS
#define SIGNALRCLIENT_API __declspec(dllexport)
#else
#define SIGNALRCLIENT_API __declspec(dllimport)
#endif