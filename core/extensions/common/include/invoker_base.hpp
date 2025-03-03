
// ----------------------------------------------------------------------------
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
// ----------------------------------------------------------------------------

#pragma once

namespace kungfu {

    struct invoker_base {
        virtual bool invoke(const char *fl, int ec, std::string errmsg, const char *ptr, size_t size) = 0;
    };

} // ns kungfu

