
// ----------------------------------------------------------------------------
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
// ----------------------------------------------------------------------------

#pragma once

#include "boost/multiprecision/cpp_dec_float.hpp"

namespace kungfu {

using double_type = boost::multiprecision::number<boost::multiprecision::cpp_dec_float<8>, boost::multiprecision::et_off >;

} // ns kungfu

