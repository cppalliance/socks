//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_COMMAND_HPP
#define BOOST_SOCKS_PROTO_COMMAND_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <boost/socks_proto/byte.hpp>
#include <iosfwd>

namespace boost {
namespace socks_proto {

/** Constants representing SOCKS commands.
*/
enum class command : byte
{
    /// CONNECT request
    connect       = 0x01,
    /// BIND request
    bind          = 0x02,
    /// UDP ASSOCIATE request
    udp_associate = 0x03,
    /// Command not supported
    unsupported   = 0xFF
};

/** Return the serialized string representing the SOCKS command
*/
BOOST_SOCKS_PROTO_DECL
string_view
to_string(command v) noexcept;

/** Format the version to an output stream.
*/
BOOST_SOCKS_PROTO_DECL
std::ostream&
operator<<(std::ostream& os, command v);

} // socks_proto
} // boost

#endif
