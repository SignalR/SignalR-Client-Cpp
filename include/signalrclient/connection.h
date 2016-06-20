#include <cpprest\http_client.h>
#include "_exports.h"
#include "transport_type.h"

namespace signalr
{
    class connection
    {
    public:
        connection(utility::string_t uri, utility::string_t queryString = U(""));
        ~connection();

        SIGNALRCLIENT_API pplx::task<void> start();
        SIGNALRCLIENT_API pplx::task<void> start(transport_type transport);

    private:
    };
}