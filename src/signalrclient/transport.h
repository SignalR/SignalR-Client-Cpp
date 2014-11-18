// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once;

#include <cpprest\base_uri.h>
#include <ppl.h>

namespace pplx = Concurrency;

namespace signalr
{
    class connection_impl;

    class transport
    {
    public:
        virtual pplx::task<void> connect(const web::uri &url) = 0;

        virtual pplx::task<void> send(const utility::string_t &data) = 0;

        virtual pplx::task<void> disconnect() = 0;

        virtual ~transport();

    protected:
        transport(std::shared_ptr<connection_impl> connection, std::function<void(const utility::string_t &)> process_response_callback);

        void process_response(const utility::string_t &message);

        std::shared_ptr<connection_impl> m_connection;

    private:
        std::function<void(const utility::string_t &)> m_process_response_callback;
    };
}