// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include <cpprest\http_client.h>
#include "signalrclient\web_exception.h"
#include "web_request.h"
#include "constants.h"

namespace signalr
{
    namespace http_sender
    {
        pplx::task<utility::string_t> get(web_request &request)
        {
            request.set_method(web::http::methods::GET);

            // TODO: set other and custom headers
            request.set_user_agent(USER_AGENT);

            return request.get_response().then([](web_response response)
            {
                if (response.status_code != 200)
                {
                    utility::ostringstream_t oss;
                    oss << _XPLATSTR("web exception: ") << response.status_code << _XPLATSTR(" ") << response.reason_phrase;
                    throw web_exception(oss.str());
                }

                return response.body;
            });
        }
    }
}