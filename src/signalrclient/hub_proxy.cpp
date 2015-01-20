// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "signalrclient\hub_proxy.h"
#include "internal_hub_proxy.h"

namespace signalr
{
    hub_proxy::hub_proxy(const std::shared_ptr<internal_hub_proxy>& proxy)
        : m_pImpl(proxy)
    { }

    // Do NOT remove this destructor. Letting the compiler generate and inline the default dtor may lead to
    // undefinded behavior since we are using an incomplete type. More details here:  http://herbsutter.com/gotw/_100/
    hub_proxy::~hub_proxy() = default;

    utility::string_t hub_proxy::get_hub_name() const
    {
        return m_pImpl->get_hub_name();
    }

    void hub_proxy::on(const utility::string_t& event_name, const std::function<void(const web::json::value &)>& handler)
    {
        return m_pImpl->on(event_name, handler);
    }

    pplx::task<web::json::value> hub_proxy::invoke_json(const utility::string_t& method_name, const web::json::value& arguments,
        const std::function<void(const web::json::value&)>& on_progress)
    {
        return m_pImpl->invoke_json(method_name, arguments, on_progress);
    }

    pplx::task<void> hub_proxy::invoke_void(const utility::string_t& method_name, const web::json::value& arguments,
        const std::function<void(const web::json::value&)>& on_progress)
    {
        return m_pImpl->invoke_void(method_name, arguments, on_progress);
    }
}