// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include <cpprest\http_client.h>

namespace signalr
{
    template<typename T>
    class connection_impl
    {
    public:
        connection_impl(const transport_factory& transport_factory)
            : m_transport_factory(transport_factory)
        { }

        connection_impl(const connection_impl<T>&) = delete;

        connection_impl<T>& operator=(const connection_impl<T>&) = delete;

        pplx::task<void> start()
        {
            return pplx::task_from_result();
        }

    private:
        const transport_factory& m_transport_factory;
    };
}
