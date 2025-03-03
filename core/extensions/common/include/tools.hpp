
// ----------------------------------------------------------------------------
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
// ----------------------------------------------------------------------------

#pragma once

#include "double_type.hpp"

#include <string>
#include <vector>

using namespace std;

namespace kungfu {

size_t num_fractions_from_double_type(const double_type &v);

double_type adjust_to_step(const double_type &v, const double_type &s, bool increase = false);
std::vector<std::string> split_string(const std::string &str, const char *sep);
std::string join_string(const std::vector<std::string> &vec, const char *sep);


double_type percents_diff(const double_type &a, const double_type &b);
double_type percents_add(const double_type &v, const double_type &p);
double_type percents_sub(const double_type &v, const double_type &p);
double_type percents_val_by_percent(const double_type &v, const double_type &p);


bool is_my_orderid(const std::string &client_order_id);
bool is_my_orderid(const char *client_order_id);

uint64_t get_current_ms_epoch();
string b2a_hex(const std::uint8_t *p, size_t n);
string hmac_sha256(const char *key, size_t klen, const char *data, size_t dlen);

} // ns kungfu

