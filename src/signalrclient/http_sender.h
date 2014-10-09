// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include <cpprest\basic_types.h>
#include <cpprest\http_client.h>
#include "signalrclient\web_response.h"

namespace signalr
{
    namespace http_sender
    {
        template<typename T>
        static pplx::task<utility::string_t> get(const web::uri &url)
        {
            get(T(url));
        }

        template<typename T>
        static pplx::task<utility::string_t> get(T &request)
        {
            request.set_method(web::http::methods::GET);

            // TODO: set headers, user agent etc.
            request.set_user_agent(_XPLATSTR(""));

            // TODO: check status code, catch exceptions

            return request.get_response().then([](web_response response) 
            {
                return response.body;
            });
        }
    }
}