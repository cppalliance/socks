//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_DETAIL_COMMAND_HPP
#define BOOST_SOCKS_PROTO_DETAIL_COMMAND_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <iosfwd>

namespace boost {
namespace socks_proto {
namespace detail {

// Constants representing SOCKS commands.
enum class command : unsigned char
{
    // CONNECT request
    connect       = 0x01,

    // BIND request
    bind          = 0x02,

    // UDP ASSOCIATE request
    udp_associate = 0x03,

    // Command not supported
    unsupported   = 0xFF
};

// Converts an integer to a known reply code.
//
// If the integer does not match a known reply code,
// command::unsupported is returned.
BOOST_SOCKS_PROTO_DECL
command
to_command(unsigned v);

// Return the serialized string representing the SOCKS command
BOOST_SOCKS_PROTO_DECL
string_view
to_string(command v) noexcept;

// Format the version to an output stream.
BOOST_SOCKS_PROTO_DECL
std::ostream&
operator<<(std::ostream& os, command v);

} // detail
} // socks_proto
} // boost

#endif
