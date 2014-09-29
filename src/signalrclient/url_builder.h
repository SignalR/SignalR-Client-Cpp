#include <cpprest\http_client.h>
#include "constants.h"

namespace signalr {
    namespace url_builder
    {
        web::uri_builder &append_client_protocol(web::uri_builder &uri_builder)
        {
            return uri_builder.append_query(_XPLATSTR("clientProtocol"), PROTOCOL);
        }

        web::uri build_negotiate(const web::uri& base_uri, const utility::string_t& query_string)
        {
            auto uri_builder = web::uri_builder(base_uri).append_path(_XPLATSTR("negotiate"));

            return append_client_protocol(uri_builder)
                .append_query(query_string)
                .to_uri();
        }
    }
}