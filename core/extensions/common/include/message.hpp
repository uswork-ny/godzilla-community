
// ----------------------------------------------------------------------------
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
// ----------------------------------------------------------------------------

#pragma once

#define __STRINGIZE_I(x) #x
#define __STRINGIZE(x) __STRINGIZE_I(x)

#define __MAKE_FILELINE \
    __FILE__ "(" __STRINGIZE(__LINE__) ")"

#define __MESSAGE(msg) \
    std::strrchr(__FILE__, '/')+1 << "(" __STRINGIZE(__LINE__) "): " << msg << std::flush

#define __MAKE_ERRMSG(res, msg) \
    res.errmsg = __MAKE_FILELINE; \
    res.errmsg += ": "; \
    res.errmsg += msg;
