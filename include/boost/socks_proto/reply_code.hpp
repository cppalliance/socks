//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_REPLY_CODE_HPP
#define BOOST_SOCKS_PROTO_REPLY_CODE_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/error.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <boost/system/is_error_code_enum.hpp>
#include <iosfwd>

namespace boost {
namespace socks_proto {

/** Reply code from SOCKS5 server to client

   @par Specification
   @li https://datatracker.ietf.org/doc/html/rfc1928#section-6

 */
enum class reply_code : uint8_t
{
    /// Succeeded
    succeeded                           = 0x00,
    /// General SOCKS server failure
    general_failure                     = 0x01,
    /// Connection not allowed by ruleset
    connection_not_allowed_by_ruleset   = 0x02,
    /// Network unreachable
    network_unreachable                 = 0x03,
    /// Host unreachable
    host_unreachable                    = 0x04,
    /// Connection refused
    connection_refused                  = 0x05,
    /// TTL expired
    ttl_expired                         = 0x06,
    /// Command not supported
    command_not_supported               = 0x07,
    /// Address type not supported
    address_type_not_supported          = 0x08,
    /// Unassigned
    unassigned                          = 0xFF,
};

/** Converts an integer to a known reply code.

    If the integer does not match a known reply code,
    @ref status::unassigned is returned.
*/
BOOST_SOCKS_PROTO_DECL
reply_code
to_reply_code(unsigned v);

} // socks_proto
} // boost

#endif
