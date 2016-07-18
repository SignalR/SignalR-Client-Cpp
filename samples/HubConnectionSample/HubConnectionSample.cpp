#include "stdafx.h"

#include <iostream>
#include <sstream>
#include "signalrclient\hub_connection.h"

void send_message(signalr::hub_proxy proxy, const utility::string_t& name, const utility::string_t& message)
{
    web::json::value args{};
    args[0] = web::json::value::string(name);
    args[1] = web::json::value(message);

    // if you get an internal compiler error uncomment the lambda below or install VS Update 4
    proxy.invoke<void>(U("send"), args/*, [](const web::json::value&){}*/)
        .then([](pplx::task<void> invoke_task)  // fire and forget but we need to observe exceptions
    {
        try
        {
            invoke_task.get();
        }
        catch (const std::exception &e)
        {
            ucout << U("Error while sending data: ") << e.what();
        }
    });
}

void chat(const utility::string_t& name)
{
    signalr::hub_connection connection{U("http://localhost:34281")};
    auto proxy = connection.create_hub_proxy(U("ChatHub"));
    proxy.on(U("broadcastMessage"), [](const web::json::value& m)
    {
        ucout << std::endl << m.at(0).as_string() << U(" wrote:") << m.at(1).as_string() << std::endl << U("Enter your message: ");
    });

    connection.start()
        .then([proxy, name]()
        {
            ucout << U("Enter your message:");
            for (;;)
            {
                utility::string_t message;
                std::getline(ucin, message);

                if (message == U(":q"))
                {
                    break;
                }

                send_message(proxy, name, message);
            }
        })
        .then([&connection]() // fine to capture by reference - we are blocking so it is guaranteed to be valid
        {
            return connection.stop();
        })
        .then([](pplx::task<void> stop_task)
        {
            try
            {
                stop_task.get();
                ucout << U("connection stopped successfully") << std::endl;
            }
            catch (const std::exception &e)
            {
                ucout << U("exception when starting or stopping connection: ") << e.what() << std::endl;
            }
        }).get();
}

int main()
{
    ucout << U("Enter your name: ");
    utility::string_t name;
    std::getline(ucin, name);

    chat(name);

    return 0;
}
