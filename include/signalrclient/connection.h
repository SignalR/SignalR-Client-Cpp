#include <cpprest\http_client.h>
#include "_exports.h"
#include "transport_type.h"

namespace signalr
{
    class connection
    {
    private:
        web::uri m_base_uri;
        utility::string_t m_querystring;

    public:
        connection(const utility::string_t& url, const utility::string_t& querystring = U(""))
            : m_base_uri(url), m_querystring(querystring)
        {
        }

        ~connection();

        SIGNALRCLIENT_API pplx::task<void> start();
        SIGNALRCLIENT_API pplx::task<void> start(transport_type transport);

    private:
    };
}