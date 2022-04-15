//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_AUTH_METHOD_HPP
#define BOOST_SOCKS_PROTO_AUTH_METHOD_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <iosfwd>

namespace boost {
namespace socks_proto {

/** Constants representing SOCKS auth_methods.
*/
enum class auth_method : unsigned char
{
    /// No authentication
    no_authentication = 0x00,
    /// Generic Security Services Application Program Interface (RFC 1961)
    gssapi = 0x01,
    /// Username/password (RFC 1929)
    userpass = 0x02,
    /// IANA: Challenge-Handshake Authentication Protocol
    challenge_handshake = 0x03,
    /// IANA: Unassigned (0x04 or 0x0A-0x7F)
    unassigned = 0x04,
    /// IANA: Challenge-Response Authentication Method
    challenge_response = 0x05,
    /// IANA: Secure Sockets Layer
    ssl = 0x06,
    /// IANA: NDS Authentication
    nds_authentication = 0x07,
    /// IANA: Multi-Authentication Framework
    multi_authentication_framework = 0x08,
    /// IANA: JSON Parameter Block
    json_parameter_block = 0x09,
    /// Methods reserved for private use (0x80-0xFE)
    private_authentication = 0x80,
    /// No acceptable method (reserved for SOCKS server reply)
    no_acceptable_method = 0xFF,
};

/** Return the serialized string representing the SOCKS auth_method
*/
BOOST_SOCKS_PROTO_DECL
string_view
to_string(auth_method v) noexcept;

/** Format the version to an output stream.
*/
BOOST_SOCKS_PROTO_DECL
std::ostream&
operator<<(std::ostream& os, auth_method v);

} // socks_proto
} // boost

#endif
