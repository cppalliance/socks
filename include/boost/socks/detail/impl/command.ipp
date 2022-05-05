//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_SOCKS_DETAIL_IMPL_COMMAND_IPP
#define BOOST_SOCKS_DETAIL_IMPL_COMMAND_IPP

#include <boost/socks/detail/command.hpp>
#include <ostream>

namespace boost {
namespace socks {
namespace detail {

command
to_command(unsigned v)
{
    switch(static_cast<command>(v))
    {
    case command::connect:
    case command::bind:
    case command::udp_associate:
        return static_cast<command>(v);
    default:
        break;
    }
    return command::unsupported;
}

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

} // detail
} // http_proto
} // boost

#endif
