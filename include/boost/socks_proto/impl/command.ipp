//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_SOCKS_PROTO_IMPL_COMMAND_IPP
#define BOOST_SOCKS_PROTO_IMPL_COMMAND_IPP

#include <boost/socks_proto/command.hpp>
#include <ostream>

namespace boost {
namespace socks_proto {

string_view
to_string(command v) noexcept
{
    switch(v)
    {
    case command::connect:
        return "CONNECT";
    case command::bind:
        return "BIND";
    case command::udp_associate:
        return "UDP ASSOCIATE";
    default:
        return "UNSUPPORTED";
    }
}

std::ostream&
operator<<(
    std::ostream& os,
    command v)
{
    os << to_string(v);
    return os;
}

} // http_proto
} // boost

#endif
