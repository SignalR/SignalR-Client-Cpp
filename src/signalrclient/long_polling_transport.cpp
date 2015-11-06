// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "long_polling_transport.h"

namespace signalr
{
    std::shared_ptr<transport> long_polling_transport::create(std::shared_ptr<web_request_factory>& web_request_factory,
        const logger& logger, const std::function<void(const utility::string_t&)>& process_response_callback,
        std::function<void(const std::exception&)> error_callback)
    {
        return std::shared_ptr<transport>(new long_polling_transport(web_request_factory, logger, process_response_callback, error_callback));
    }

    long_polling_transport::long_polling_transport(std::shared_ptr<web_request_factory>& web_request_factory,
        const logger& logger, const std::function<void(const utility::string_t &)>& process_response_callback,
        std::function<void(const std::exception&)> error_callback)
        : transport(logger, process_response_callback, error_callback), m_web_request_factory(web_request_factory)
    {
    }

    long_polling_transport::~long_polling_transport()
    {
    }

    pplx::task<void> long_polling_transport::connect(const web::uri& /*url*/)
    {
        return pplx::task_from_exception<void>(std::runtime_error("not implemented"));
    }

    pplx::task<void> long_polling_transport::send(const utility::string_t& /*data*/)
    {
        return pplx::task_from_exception<void>(std::runtime_error("not implemented"));
    }

    pplx::task<void> long_polling_transport::disconnect()
    {
        return pplx::task_from_exception<void>(std::runtime_error("not implemented"));
    }

    transport_type long_polling_transport::get_transport_type() const
    {
        return transport_type::long_polling;
    }
}