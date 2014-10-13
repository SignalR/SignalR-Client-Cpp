// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "web_request_factory.h"
#include "web_request_stub.h"

using namespace signalr;

template<typename T>
class test_web_request_factory : public web_request_factory<T>
{
private:
    T m_web_request;

public:

    test_web_request_factory(const T &web_request)
        : m_web_request(web_request)
    { }

    T create_web_request(const web::uri &) const
    {
        return m_web_request;
    }
};