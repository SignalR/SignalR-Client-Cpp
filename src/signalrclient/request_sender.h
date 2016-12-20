// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "cpprest/base_uri.h"
#include "cpprest/http_client.h"
#include "signalrclient/transport_type.h"
#include "web_request_factory.h"
#include "negotiation_response.h"


namespace signalr
{
    namespace request_sender
    {
        pplx::task<negotiation_response> negotiate(web_request_factory& request_factory, const web::uri &base_url,
            const utility::string_t& connection_data, const utility::string_t& query_string,
            const std::unordered_map<utility::string_t, utility::string_t> &headers,
            const web::http::client::http_client_config &client_config = const web::http::client::http_client_config{});
        pplx::task<void> start(web_request_factory& request_factory, const web::uri& base_url, transport_type transport,
            const utility::string_t& connection_token, const utility::string_t& connection_data, const utility::string_t& query_string,
            const std::unordered_map<utility::string_t, utility::string_t>& headers,
            const web::http::client::http_client_config &client_config = const web::http::client::http_client_config{});
        pplx::task<utility::string_t> abort(web_request_factory& request_factory, const web::uri& base_url, transport_type transport,
            const utility::string_t& connection_token, const utility::string_t& connection_data, const utility::string_t& query_string,
            const std::unordered_map<utility::string_t, utility::string_t>& headers,
            const web::http::client::http_client_config &client_config = const web::http::client::http_client_config{});
    }
}