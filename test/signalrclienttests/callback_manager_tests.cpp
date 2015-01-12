// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "callback_manager.h"

using namespace signalr;
using namespace web;

TEST(callback_manager_register_callback, register_returns_unique_callback_ids)
{
    auto callback_mgr = callback_manager{ json::value::object() };
    auto callback_id1 = callback_mgr.register_callback([](const json::value&){});
    auto callback_id2 = callback_mgr.register_callback([](const web::json::value&){});

    ASSERT_NE(callback_id1, callback_id2);
}

TEST(callback_manager_complete_callback, complete_callback_invokes_callback_and_returns_true_for_valid_callback_id)
{
    auto callback_mgr = callback_manager{ json::value::object() };

    utility::string_t callback_argument{_XPLATSTR("")};

    auto callback_id = callback_mgr.register_callback(
        [&callback_argument](const json::value& argument)
        {
            callback_argument = argument.serialize();
        });

    auto callback_found = callback_mgr.complete_callback(callback_id, json::value::number(42));

    ASSERT_TRUE(callback_found);
    ASSERT_EQ(_XPLATSTR("42"), callback_argument);
}

TEST(callback_manager_complete_callback, complete_callback_returns_false_for_invalid_callback_id)
{
    auto callback_mgr = callback_manager{ json::value::object() };
    auto callback_found = callback_mgr.complete_callback(_XPLATSTR("42"), json::value::object());

    ASSERT_FALSE(callback_found);
}

TEST(callback_manager_remove, remove_removes_callback_and_returns_true_for_valid_callback_id)
{
    auto callback_called = false;

    {
        auto callback_mgr = callback_manager{ json::value::object() };

        auto callback_id = callback_mgr.register_callback(
            [&callback_called](const json::value&)
        {
            callback_called = true;
        });

        ASSERT_TRUE(callback_mgr.remove_callback(callback_id));
    }

    ASSERT_FALSE(callback_called);
}

TEST(callback_manager_remove, remove_returns_false_for_invalid_callback_id)
{
    auto callback_mgr = callback_manager{ json::value::object() };
    ASSERT_FALSE(callback_mgr.remove_callback(_XPLATSTR("42")));
}

TEST(callback_manager_clear, clear_invokes_all_callbacks)
{
    auto callback_mgr = callback_manager{ json::value::object() };
    auto invocation_count = 0;

    for (auto i = 0; i < 10; i++)
    {
        callback_mgr.register_callback(
            [&invocation_count](const json::value& argument)
        {
            invocation_count++;
            ASSERT_EQ(_XPLATSTR("42"), argument.serialize());
        });
    }

    callback_mgr.clear(json::value::number(42));

    ASSERT_EQ(10, invocation_count);
}

TEST(callback_manager_dtor, clear_invokes_all_callbacks)
{
    auto invocation_count = 0;
    bool parameter_correct = true;

    {
        auto callback_mgr = callback_manager{ json::value::number(42) };
        for (auto i = 0; i < 10; i++)
        {
            callback_mgr.register_callback(
                [&invocation_count, &parameter_correct](const json::value& argument)
            {
                invocation_count++;
                parameter_correct &= argument.serialize() == _XPLATSTR("42");
            });
        }
    }

    ASSERT_EQ(10, invocation_count);
    ASSERT_TRUE(parameter_correct);
}
