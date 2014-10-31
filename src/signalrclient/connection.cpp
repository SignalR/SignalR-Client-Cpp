// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "signalrclient\connection.h"
#include "signalrclient\transport_type.h"
#include "signalrclient\web_request_factory.h"
#include "signalrclient\transport_factory.h"
#include "connection_impl.h"

namespace signalr
{
    connection::connection(const utility::string_t& url, const utility::string_t& querystring)
    {
        m_pImpl = new connection_impl(url, querystring, m_web_request_factory, m_transport_factory);
    }

    connection::~connection()
    {
        delete m_pImpl;
    }

    pplx::task<void> connection::start()
    {
        return m_pImpl->start();
    }
}

