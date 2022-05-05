//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_SOCKS_DETAIL_IMPL_AUTH_METHOD_IPP
#define BOOST_SOCKS_DETAIL_IMPL_AUTH_METHOD_IPP

#include <boost/socks/detail/auth_method.hpp>
#include <ostream>

namespace boost {
namespace socks {
namespace detail {

string_view
to_string(auth_method v) noexcept
{
    switch(v)
    {
    case auth_method::no_authentication:
        return "No authentication";
    case auth_method::gssapi:
        return "Generic Security Services Application Program Interface";
    case auth_method::userpass:
        return "Username/password";
    case auth_method::challenge_handshake:
        return "Challenge-Handshake Authentication Protocol";
    case auth_method::challenge_response:
        return "Challenge-Response Authentication Method";
    case auth_method::ssl:
        return "Secure Sockets Layer";
    case auth_method::nds_authentication:
        return "NDS Authentication";
    case auth_method::multi_authentication_framework:
        return "Multi-Authentication Framework";
    case auth_method::json_parameter_block:
        return "JSON Parameter Block";
    case auth_method::no_acceptable_method:
        return "No acceptable method";
    default:
        if (static_cast<unsigned char>(v) <= 0x7F)
            return "Unassigned";
        return "Methods reserved for private use";
    }
}

std::ostream&
operator<<(
    std::ostream& os,
    auth_method v)
{
    os << to_string(v);
    return os;
}

} // detail
} // http_proto
} // boost

#endif
