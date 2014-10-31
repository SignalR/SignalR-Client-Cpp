// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <functional>
#include <cpprest\ws_client.h>

using namespace web::experimental;

class test_websocket_client
{
public:
    test_websocket_client();

    pplx::task<void> connect(const web::uri &url);

    pplx::task<void> send(web_sockets::client::websocket_outgoing_message msg);

    pplx::task<web_sockets::client::websocket_incoming_message> receive();

    pplx::task<void> close();

    void set_connect_function(std::function<pplx::task<void>(const web::uri &url)> connect_function);

    void set_send_function(std::function<pplx::task<void>(web_sockets::client::websocket_outgoing_message)> send_function);

    void set_receive_function(std::function<pplx::task<web_sockets::client::websocket_incoming_message>()> receive_function);

    void set_close_function(std::function<pplx::task<void>()> close_function);

private:
    std::function<pplx::task<void>(const web::uri &url)> m_connect_function;

    std::function<pplx::task<void>(web_sockets::client::websocket_outgoing_message)> m_send_function;

    std::function<pplx::task<web_sockets::client::websocket_incoming_message>()> m_receive_function;

    std::function<pplx::task<void>()> m_close_function;
};