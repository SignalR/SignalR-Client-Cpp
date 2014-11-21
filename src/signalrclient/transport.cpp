// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "transport.h"
#include "connection_impl.h"

namespace signalr
{
    transport::transport(std::shared_ptr<connection_impl> connection, std::function<void(const utility::string_t&)> process_response_callback)
        : m_connection(connection), m_process_response_callback(process_response_callback)
    {}

    // Do NOT remove this destructor. Letting the compiler generate and inline the default dtor may lead to
    // undefinded behavior since we are using an incomplete type. More details here:  http://herbsutter.com/gotw/_100/ 
    transport::~transport()
    { }

    void transport::process_response(const utility::string_t &message)
    {
        m_process_response_callback(message);
    }
}