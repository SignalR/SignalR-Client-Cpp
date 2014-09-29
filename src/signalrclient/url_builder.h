#include <cpprest\http_client.h>
#include "constants.h"

using namespace web;

namespace signalr {
    namespace url_builder
    {
        uri_builder build_uri(const utility::string_t &command, const utility::string_t &query_string)
        {
            return uri_builder(command)
                .append_query(_XPLATSTR("clientProtocol"), PROTOCOL)
                .append_query(query_string);
        }

        uri build_negotiate(const utility::string_t &query_string)
        {
            return build_uri(_XPLATSTR("negotiate"), query_string).to_uri();
        }
    }
}