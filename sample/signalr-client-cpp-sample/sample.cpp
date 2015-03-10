// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

// sample.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <string>
#include "cpprest\details\basic_types.h"
#include "cpprest\json.h" 
#include "connection.h"
#include "hub_connection.h"

using namespace signalr;
using namespace utility;

class console_log_writer : public log_writer
{
public:
	void __cdecl write(const utility::string_t &entry) override
	{
		std::wcout << entry << std::endl;
	}
};

void runRawConnection()
{
	std::shared_ptr<log_writer> Log_writer(std::make_shared<console_log_writer>());

	connection conn(L"http://localhost:8081/raw-connection", L"", trace_level::all, Log_writer);

	conn.set_message_received([&conn](const string_t& payload)
	{
		std::wcout << "Received message: " << payload << std::endl;
	});

	pplx::task_completion_event<void> tce;

	conn.start().then([&conn]()
	{
		web::json::value obj;
		obj[L"type"] = web::json::value::number(1);
		std::string str;

		do
		{
			//send message
			getline(std::cin, str);
			obj[L"value"] = web::json::value::string(conversions::to_string_t(str));

			conn.send(obj.serialize()).then([](pplx::task<void> send_task)
			{
				try
				{
					send_task.get();
				}
				catch (const std::exception &e)
				{
					std::cout << "Error while sending data: " << e.what();
				}
			});

		} while (str != "Q");

	}).then([&conn](pplx::task<void> previous_task)
	{
		try
		{
			previous_task.get();
		}
		catch (const std::exception &e)
		{
			std::cout << "exception: " << e.what() << std::endl;
		}

		return conn.stop();

	}).then([tce](pplx::task<void> stop_task)
	{
		try
		{
			stop_task.get();
			std::cout << "connection stopped successfully" << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cout << "exception when closing connection: " << e.what() << std::endl;
		}

		tce.set();
	});

	pplx::task<void>(tce).get();
}

void runHubConnection()
{
	std::shared_ptr<log_writer> Log_writer(std::make_shared<console_log_writer>());

	hub_connection hubConn(L"http://localhost:8081/", L"", trace_level::all, Log_writer);

	hub_proxy hubProxy = hubConn.create_hub_proxy(L"HubConnectionAPI");

	hubProxy.on(L"displayMessage", [](const web::json::value& arguments)
	{
		std::wcout << "Received message: " << arguments.serialize() << std::endl;
	});

	pplx::task_completion_event<void> tce;

	hubConn.start().then([&hubProxy]()
	{
		web::json::value obj{};
		std::wstring str;

		do
		{
			//send message
			getline(std::wcin, str);
			obj[0] = web::json::value(str);

			hubProxy.invoke<web::json::value>(L"displayMessageAll", obj)
				.then([](pplx::task<web::json::value> send_task)
			{
				try
				{
					auto test = send_task.get();
				}
				catch (const std::exception &e)
				{
					std::cout << "Error while sending data: " << e.what();
				}
			});
		} while (str != L"Q");

	}).then([&hubConn](pplx::task<void> previous_task)
	{
		try
		{
			previous_task.get();
		}
		catch (const std::exception &e)
		{
			std::cout << "exception: " << e.what() << std::endl;
		}

		return hubConn.stop();

	}).then([tce](pplx::task<void> stop_task)
	{
		try
		{
			stop_task.get();
			std::cout << "connection stopped successfully" << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cout << "exception when closing connection: " << e.what() << std::endl;
		}

		tce.set();
	});

	pplx::task<void>(tce).get();
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "1.	runRawConnection()" << std::endl;
	std::cout << "2.	runHubConnection()" << std::endl;
	std::cout << "Input run option:" << std::endl;
	int i;
	std::cin >> i;

	if (i == 1)
	{
		runRawConnection();
	}

	if (i == 2)
	{
		runHubConnection();
	}
	return 0;
}

