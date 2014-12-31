// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "request_sender.h"
#include "http_sender.h"
#include "url_builder.h"

namespace signalr
{
    namespace request_sender
    {
        pplx::task<negotiation_response> negotiate(web_request_factory& request_factory, const web::uri& base_url,
            const utility::string_t& connection_data, const utility::string_t& query_string)
        {
            auto negotiate_url = url_builder::build_negotiate(base_url, connection_data, query_string);
            auto request = request_factory.create_web_request(negotiate_url);

            return http_sender::get(*request)
                .then([](utility::string_t body)
            {
                auto negotiation_response_json = web::json::value::parse(body);
                negotiation_response response{
                    negotiation_response_json[_XPLATSTR("ConnectionId")].as_string(),
                    negotiation_response_json[_XPLATSTR("ConnectionToken")].as_string(),
                    (int)(negotiation_response_json[_XPLATSTR("DisconnectTimeout")].as_double() * 1000),
                    !negotiation_response_json[_XPLATSTR("KeepAliveTimeout")].is_null()
                        ? (int)(negotiation_response_json[_XPLATSTR("KeepAliveTimeout")].as_double() * 1000)
                        : -1,
                    negotiation_response_json[_XPLATSTR("ProtocolVersion")].as_string(),
                    negotiation_response_json[_XPLATSTR("TryWebSockets")].as_bool(),
                    (int)(negotiation_response_json[_XPLATSTR("TransportConnectTimeout")].as_double() * 1000)
                };

                return response;
            });
        }

        pplx::task<void> start(web_request_factory& request_factory, const web::uri &base_url, transport_type transport,
            const utility::string_t& connection_data, const utility::string_t& connection_token, const utility::string_t &query_string)
        {
            auto start_url = url_builder::build_start(base_url, transport, connection_token, connection_data, query_string);
            auto request = request_factory.create_web_request(start_url);

            return http_sender::get(*request)
                .then([](utility::string_t body)
            {
                auto start_response_json = web::json::value::parse(body);

                if (start_response_json[_XPLATSTR("Response")].is_null() ||
                    start_response_json[_XPLATSTR("Response")].as_string() != _XPLATSTR("started"))
                {
                    throw std::runtime_error(std::string("start request failed due to unexpected response from the server: ")
                            .append(utility::conversions::to_utf8string(body)));
                }
            });
        }

        pplx::task<utility::string_t> abort(web_request_factory& request_factory, const web::uri &base_url, transport_type transport,
            const utility::string_t& connection_token, const utility::string_t& connection_data, const utility::string_t &query_string)
        {
            auto abort_url = url_builder::build_abort(base_url, transport, connection_token, connection_data, query_string);
            auto request = request_factory.create_web_request(abort_url);

            return http_sender::get(*request);
        }
    }
}