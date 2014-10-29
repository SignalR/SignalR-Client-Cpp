// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\basic_types.h>
#include <cpprest\base_uri.h>
#include "web_response.h"

namespace signalr
{
    class web_request
    {
    private:
        web::uri m_url;
        utility::string_t m_method;
        utility::string_t m_user_agent_string;

    public:
        web_request(const web::uri &url) : m_url(url)
        {}

        virtual void set_method(const utility::string_t &method);
        virtual void set_user_agent(const utility::string_t &user_agent_string);

        virtual pplx::task<web_response> get_response();
    };
}