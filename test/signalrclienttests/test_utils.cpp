// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "cpprest\basic_types.h"

utility::string_t remove_date_from_log_entry(const utility::string_t &log_entry)
{
    // dates are ISO 8601 (e.g. `2014-11-13T06:05:29.452066Z`)
    auto date_end_index = log_entry.find_first_of(_XPLATSTR("Z")) + 1;

    // date is followed by a whitespace hence +1
    return log_entry.substr(date_end_index + 1);
}