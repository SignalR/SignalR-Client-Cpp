// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "transport.h"
#include "logger.h"
#include "web_request_factory.h"

namespace signalr
{
    class long_polling_transport : public transport, public std::enable_shared_from_this<long_polling_transport>
    {
    public:
        static std::shared_ptr<transport> create(std::shared_ptr<web_request_factory>& web_request_factory,
            const logger& logger, const std::function<void(const utility::string_t&)>& process_response_callback,
            std::function<void(const std::exception&)> error_callback);

        ~long_polling_transport();

        long_polling_transport(const long_polling_transport&) = delete;

        long_polling_transport& operator=(const long_polling_transport&) = delete;

        pplx::task<void> connect(const web::uri& url) override;

        pplx::task<void> send(const utility::string_t& data) override;

        pplx::task<void> disconnect() override;

        transport_type get_transport_type() const override;

    private:
        long_polling_transport(std::shared_ptr<web_request_factory>& web_request_factory,
            const logger& logger, const std::function<void(const utility::string_t &)>& process_response_callback,
            std::function<void(const std::exception&)> error_callback);

        std::shared_ptr<web_request_factory> m_web_request_factory;
    };
}