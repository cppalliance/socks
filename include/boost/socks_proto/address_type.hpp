//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_ADDRESS_TYPE_HPP
#define BOOST_SOCKS_PROTO_ADDRESS_TYPE_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <boost/socks_proto/byte.hpp>
#include <iosfwd>

namespace boost {
namespace socks_proto {

/** Constants representing SOCKS address types.
*/
enum class address_type : byte
{
    /// IP V4 address
    ip_v4       = 0x01,
    /// Domain name
    domain_name = 0x03,
    /// IP V6 address
    ip_v6       = 0x04
};

/** Return the serialized string representing the SOCKS address_type
*/
BOOST_SOCKS_PROTO_DECL
string_view
to_string(address_type v) noexcept;

/** Format the address_type to an output stream.
*/
BOOST_SOCKS_PROTO_DECL
std::ostream&
operator<<(std::ostream& os, address_type v);

} // socks_proto
} // boost

#endif
