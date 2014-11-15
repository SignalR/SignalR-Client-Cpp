// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "logger.h"
#include <cpprest\asyncrt_utils.h>


namespace signalr
{
    logger::logger(std::shared_ptr<log_writer> log_writer, trace_level trace_level)
        : m_log_writer(log_writer), m_trace_level(trace_level)
    { }

    void logger::log(trace_level level, const utility::string_t& entry)
    {
        if ((level & m_trace_level) != trace_level::none)
        {
            try
            {
                utility::ostringstream_t os;
                os << utility::datetime::utc_now().to_string(utility::datetime::date_format::ISO_8601) << _XPLATSTR(" ") << entry << std::endl;
                m_log_writer->write(os.str());
            }
            catch (const std::exception &e)
            {
                ucerr << _XPLATSTR("error occurred when logging: ") << utility::conversions::to_string_t(e.what())
                    << std::endl << _XPLATSTR("    entry: ") << entry << std::endl;
            }
            catch (...)
            {
                ucerr << _XPLATSTR("unknown error occurred when logging") << std::endl << _XPLATSTR("    entry: ") << entry << std::endl;
            }
        }
    }
}