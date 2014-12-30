// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "test_utils.h"
#include "test_websocket_client.h"
#include "test_web_request_factory.h"

using namespace signalr;

utility::string_t remove_date_from_log_entry(const utility::string_t &log_entry)
{
    // dates are ISO 8601 (e.g. `2014-11-13T06:05:29.452066Z`)
    auto date_end_index = log_entry.find_first_of(_XPLATSTR("Z")) + 1;

    // date is followed by a whitespace hence +1
    return log_entry.substr(date_end_index + 1);
}

std::shared_ptr<websocket_client> create_test_websocket_client(std::function<pplx::task<std::string>()> receive_function,
    std::function<pplx::task<void>(const utility::string_t &msg)> send_function,
    std::function<pplx::task<void>(const web::uri &url)> connect_function,
    std::function<pplx::task<void>()> close_function)
{
    auto websocket_client = std::make_shared<test_websocket_client>();
    websocket_client->set_receive_function(receive_function);
    websocket_client->set_send_function(send_function);
    websocket_client->set_connect_function(connect_function);
    websocket_client->set_close_function(close_function);

    return websocket_client;
}

std::unique_ptr<web_request_factory> create_test_web_request_factory()
{
    return std::make_unique<test_web_request_factory>([](const web::uri& url)
    {
        auto response_body =
            url.path() == _XPLATSTR("/negotiate")
            ? _XPLATSTR("{\"Url\":\"/signalr\", \"ConnectionToken\" : \"A==\", \"ConnectionId\" : \"f7707523-307d-4cba-9abf-3eef701241e8\", ")
            _XPLATSTR("\"KeepAliveTimeout\" : 20.0, \"DisconnectTimeout\" : 30.0, \"ConnectionTimeout\" : 110.0, \"TryWebSockets\" : true, ")
            _XPLATSTR("\"ProtocolVersion\" : \"1.4\", \"TransportConnectTimeout\" : 5.0, \"LongPollDelay\" : 0.0}")
            : url.path() == _XPLATSTR("/start")
            ? _XPLATSTR("{\"Response\":\"started\" }")
            : _XPLATSTR("");

        return std::unique_ptr<web_request>(new web_request_stub((unsigned short)200, _XPLATSTR("OK"), response_body));
    });
}