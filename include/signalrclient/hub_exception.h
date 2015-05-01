// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <stdexcept>
#include "cpprest/details/basic_types.h"
#include "cpprest/json.h"
#include "cpprest/asyncrt_utils.h"

namespace signalr
{
    class hub_exception : public std::runtime_error
    {
    public:
        hub_exception(const utility::string_t &what, const web::json::value& error_data)
            : runtime_error(utility::conversions::to_utf8string(what)), m_error_data(error_data)
        {}

        web::json::value error_data() const
        {
            return m_error_data;
        }

    private:
        web::json::value m_error_data;
    };
}