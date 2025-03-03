
// ----------------------------------------------------------------------------
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
// ----------------------------------------------------------------------------

#include "../include/tools.hpp"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <chrono>
#include <queue>
#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace kungfu {

size_t num_fractions_from_double_type(const double_type &v) {
    if ( v == 1 ) return 1u;

    double_type r{v};
    size_t n = 0u;
    for ( ; r != 1; ++n ) {
        r *= 10;
    }

    return n;
}

double_type adjust_to_step(const double_type &v, const double_type &s, bool increase) {
    std::int64_t step1 = (v * 100000000).convert_to<std::int64_t>();
    std::int64_t step2 = step1 % (s * 100000000).convert_to<std::int64_t>();
    std::int64_t step3 = step1 - step2;
    double_type res    = double_type(step3) / 100000000;
    res += (increase ? s : 0.0);

    return res;
}

/*************************************************************************************************/

std::vector<string> split_string(const string &str, const char *sep) {
    std::vector<string> res{};

    boost::algorithm::split(res, str, boost::is_any_of(sep));
    for ( auto &it: res ) {
        boost::algorithm::trim(it);
    }

    return res;
}

string join_string(const std::vector<string> &vec, const char *sep) {
    return boost::algorithm::join(vec, sep);
}

/*************************************************************************************************/

double_type percents_diff(const double_type &a, const double_type &b) {
    if ( a == b ) return 0;

    return ((b - a) / a) * 100;
}

double_type percents_add(const double_type &v, const double_type &p) {
    if ( p == 0 ) return v;

    return v + ((v / 100) * p);
}

double_type percents_sub(const double_type &v, const double_type &p) {
    if ( p == 0 ) return v;

    return v - ((v / 100) * p);
}

double_type percents_val_by_percent(const double_type &v, const double_type &p) {
    return (v / 100) * p;
}

/*************************************************************************************************/

bool is_my_orderid(const char *client_order_id) {
    const char *start = client_order_id;
    const char *p = std::strchr(client_order_id, '-');
    if ( !p ) {
        return false;
    }

#define __STRCMP_CSTR(x) std::strncmp(start, x, sizeof(x)-1) == 0

    const auto len = p - start;
    switch ( len ) {
        case 3: return __STRCMP_CSTR("FIX");
        case 4: return __STRCMP_CSTR("SRSI");
        case 5: return __STRCMP_CSTR("AROON") || __STRCMP_CSTR("SLOSS") || __STRCMP_CSTR("TAKEP");
        case 6: return __STRCMP_CSTR("SRSI_U") || __STRCMP_CSTR("SRSI_D") || __STRCMP_CSTR("SRSI_C");
        case 7: return __STRCMP_CSTR("HOLEBUY") || __STRCMP_CSTR("INASELL");
        case 8: return __STRCMP_CSTR("CYCLEBUY") || __STRCMP_CSTR("SPREDBUY") || __STRCMP_CSTR("SLOSSBUY");
        default: return false;
    }

#undef __STRCMP_CSTR

    return false;
}

bool is_my_orderid(const string& client_order_id) {
    return is_my_orderid(client_order_id.c_str());
}


std::uint64_t get_current_ms_epoch() {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    ).count());
}

string b2a_hex(const std::uint8_t *p, size_t n) {
    static const char hex[] = "0123456789abcdef";
    string res;
    res.reserve(n * 2);

    for ( auto end = p + n; p != end; ++p ) {
        const std::uint8_t v = (*p);
        res += hex[(v >> 4) & 0x0F];
        res += hex[v & 0x0F];
    }

    return res;
}

string hmac_sha256(const char *key, size_t klen, const char *data, size_t dlen) {
    std::uint8_t digest[EVP_MAX_MD_SIZE];
    std::uint32_t dilen{};

    auto p = ::HMAC(
            ::EVP_sha256()
            ,key
            ,klen
            ,(std::uint8_t *)data
            ,dlen
            ,digest
            ,&dilen
    );
    assert(p);

    return b2a_hex(digest, dilen);
}

} // ns kungfu
