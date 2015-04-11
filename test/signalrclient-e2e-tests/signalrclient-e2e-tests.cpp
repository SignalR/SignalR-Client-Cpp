// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include <vector>
#include "cpprest/details/basic_types.h"
#include "test_utils.h"

int wmain(int argc, utility::char_t* argv[])
{
    get_url(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    return 0;
}