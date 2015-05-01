// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "signalrclient/hub_proxy.h"
#include "internal_hub_proxy.h"

namespace signalr
{
    hub_proxy::hub_proxy()
    { }

    hub_proxy::hub_proxy(const hub_proxy& other)
        : m_pImpl(other.m_pImpl)
    { }

    hub_proxy::hub_proxy(const hub_proxy && other)
        : m_pImpl(std::move(other.m_pImpl))
    { }

    hub_proxy::hub_proxy(const std::shared_ptr<internal_hub_proxy>& proxy)
        : m_pImpl(proxy)
    { }

    // Do NOT remove this destructor. Letting the compiler generate and inline the default dtor may lead to
    // undefinded behavior since we are using an incomplete type. More details here:  http://herbsutter.com/gotw/_100/
    hub_proxy::~hub_proxy() = default;

    utility::string_t hub_proxy::get_hub_name() const
    {
        if (!m_pImpl)
        {
            throw std::runtime_error("get_hub_name() cannot be called on uninitialized hub_proxy instance");
        }

        return m_pImpl->get_hub_name();
    }

    void hub_proxy::on(const utility::string_t& event_name, const method_invoked_handler& handler)
    {
        if (!m_pImpl)
        {
            throw std::runtime_error("on() cannot be called on uninitialized hub_proxy instance");
        }

        return m_pImpl->on(event_name, handler);
    }

    pplx::task<web::json::value> hub_proxy::invoke_json(const utility::string_t& method_name, const web::json::value& arguments,
        const on_progress_handler& on_progress)
    {
        if (!m_pImpl)
        {
            throw std::runtime_error("invoke() cannot be called on uninitialized hub_proxy instance");
        }

        return m_pImpl->invoke_json(method_name, arguments, on_progress);
    }

    pplx::task<void> hub_proxy::invoke_void(const utility::string_t& method_name, const web::json::value& arguments,
        const on_progress_handler& on_progress)
    {
        if (!m_pImpl)
        {
            throw std::runtime_error("invoke() cannot be called on uninitialized hub_proxy instance");
        }

        return m_pImpl->invoke_void(method_name, arguments, on_progress);
    }

    hub_proxy& hub_proxy::operator=(const hub_proxy& other)
    {
        if (this != &other)
        {
            m_pImpl = other.m_pImpl;
        }

        return *this;
    }

    hub_proxy& hub_proxy::operator=(const hub_proxy&& other)
    {
        if (this != &other)
        {
            m_pImpl = std::move(other.m_pImpl);
        }

        return *this;
    }
}
