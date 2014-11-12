// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "signalrclient\connection.h"
#include "signalrclient\transport_type.h"
#include "connection_impl.h"

namespace signalr
{
    connection::connection(const utility::string_t& url, const utility::string_t& querystring)
    {
        m_pImpl = new connection_impl(url, querystring);
    }

    connection::~connection()
    {
        delete m_pImpl;
    }

    pplx::task<void> connection::start()
    {
        return m_pImpl->start();
    }

    connection_state connection::get_connection_state() const
    {
        return m_pImpl->get_connection_state();
    }
}

