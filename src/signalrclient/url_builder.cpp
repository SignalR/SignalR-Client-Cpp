// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "constants.h"
#include <cpprest\http_client.h>
#include "signalrclient\transport_type.h"

namespace signalr
{
    namespace url_builder
    {
        utility::string_t get_transport_name(transport_type transport)
        {
            assert(transport == transport_type::websockets || transport == transport_type::long_polling);

            return transport == transport_type::websockets
                ? _XPLATSTR("webSockets")
                : _XPLATSTR("longPolling");
        }

        void append_transport(web::uri_builder &builder, transport_type transport)
        {
            if (transport > static_cast<transport_type>(-1))
            {
                builder.append_query(_XPLATSTR("transport"), get_transport_name(transport));
            }
        }

        void append_connection_token(web::uri_builder &builder, const utility::string_t &connection_token)
        {
            if (connection_token != _XPLATSTR(""))
            {
                builder.append_query(_XPLATSTR("connectionToken"), connection_token, /* do_encoding */ true);
            }
        }

        web::uri_builder &convert_to_websocket_url(web::uri_builder &builder, transport_type transport)
        {
            if (transport == transport_type::websockets)
            {
                if (builder.scheme() == _XPLATSTR("https"))
                {
                    builder.set_scheme(utility::string_t(_XPLATSTR("wss")));
                }
                else
                {
                    builder.set_scheme(utility::string_t(_XPLATSTR("ws")));
                }
            }

            return builder;
        }

        web::uri_builder build_uri(const web::uri &base_url, const utility::string_t &command, transport_type transport,
            const utility::string_t &connection_token, const utility::string_t &query_string)
        {
            web::uri_builder builder(base_url); 
            builder.append_path(command);
            append_transport(builder, transport);
            builder.append_query(_XPLATSTR("clientProtocol"), PROTOCOL);
            append_connection_token(builder, connection_token);
            return builder.append_query(query_string);
        }

        web::uri build_negotiate(const web::uri &base_url, const utility::string_t &query_string)
        {
            auto test = build_uri(base_url, _XPLATSTR("negotiate"), static_cast<transport_type>(-1),
                /*connection_token*/ _XPLATSTR(""), query_string).to_uri();

            return build_uri(base_url, _XPLATSTR("negotiate"), static_cast<transport_type>(-1),
                /*connection_token*/ _XPLATSTR(""), query_string).to_uri();
        }

        web::uri build_connect(const web::uri &base_url, transport_type transport,
            const utility::string_t &connection_token, const utility::string_t &query_string)
        {
            auto builder = build_uri(base_url, _XPLATSTR("connect"), transport, connection_token, query_string);
            return convert_to_websocket_url(builder, transport).to_uri();
        }
    }
}