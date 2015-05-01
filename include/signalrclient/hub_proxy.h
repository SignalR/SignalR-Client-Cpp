// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "_exports.h"
#include <memory>
#include <memory>
#include <functional>
#include "pplx/pplxtasks.h"
#include "cpprest/details/basic_types.h"
#include "cpprest/json.h"

namespace signalr
{
    class internal_hub_proxy;

    class hub_proxy
    {
    public:
        typedef std::function<void __cdecl (const web::json::value&)> method_invoked_handler;
        typedef std::function<void __cdecl (const web::json::value&)> on_progress_handler;

        explicit hub_proxy(const std::shared_ptr<internal_hub_proxy>& proxy);

        SIGNALRCLIENT_API hub_proxy();

        SIGNALRCLIENT_API hub_proxy(const hub_proxy& other);

        SIGNALRCLIENT_API hub_proxy(const hub_proxy&& other);

        SIGNALRCLIENT_API ~hub_proxy();

        SIGNALRCLIENT_API hub_proxy& __cdecl operator=(const hub_proxy& other);
        SIGNALRCLIENT_API hub_proxy& __cdecl operator=(const hub_proxy&& other);

        SIGNALRCLIENT_API utility::string_t __cdecl get_hub_name() const;

        SIGNALRCLIENT_API void __cdecl on(const utility::string_t& event_name, const method_invoked_handler& handler);

        template<typename T>
        pplx::task<T> invoke(const utility::string_t& method_name, const on_progress_handler& on_progress = [](const web::json::value&){})
        {
            static_assert(std::is_same<web::json::value, T>::value, "only web::json::value allowed");
            return invoke_json(method_name, web::json::value().array(), on_progress);
        }

        template<>
        pplx::task<void> invoke<void>(const utility::string_t& method_name, const on_progress_handler& on_progress)
        {
            return invoke_void(method_name, web::json::value().array(), on_progress);
        }

        template<typename T>
        pplx::task<T> invoke(const utility::string_t& method_name, const web::json::value& arguments,
            const on_progress_handler& on_progress = [](const web::json::value&){})
        {
            static_assert(std::is_same<web::json::value, T>::value, "only web::json::value allowed");
            return invoke_json(method_name, arguments, on_progress);
        }

        template<>
        pplx::task<void> invoke<void>(const utility::string_t& method_name, const web::json::value& arguments,
            const on_progress_handler& on_progress)
        {
            return invoke_void(method_name, arguments, on_progress);
        }

    private:
        std::shared_ptr<internal_hub_proxy> m_pImpl;

        SIGNALRCLIENT_API pplx::task<web::json::value> __cdecl invoke_json(const utility::string_t& method_name, const web::json::value& arguments,
            const on_progress_handler& on_progress);
        SIGNALRCLIENT_API pplx::task<void> __cdecl invoke_void(const utility::string_t& method_name, const web::json::value& arguments,
            const on_progress_handler& on_progress);
    };
}