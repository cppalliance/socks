//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_SOCKS_PROTO_IMPL_ADDRESS_TYPE_IPP
#define BOOST_SOCKS_PROTO_IMPL_ADDRESS_TYPE_IPP

#include <boost/socks_proto/address_type.hpp>
#include <ostream>

namespace boost {
namespace socks_proto {

string_view
to_string(address_type v) noexcept
{
    switch(v)
    {
    case address_type::ip_v4:
        return "IPv4";
    case address_type::domain_name:
        return "Domain name";
    case address_type::ip_v6:
        return "IPv6";
    }
    return "UNKNOWN";
}

std::ostream&
operator<<(
    std::ostream& os,
    address_type v)
{
    os << to_string(v);
    return os;
}

} // http_proto
} // boost

#endif
