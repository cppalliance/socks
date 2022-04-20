//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_REPLY_CODE_V4_HPP
#define BOOST_SOCKS_PROTO_REPLY_CODE_V4_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/error.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <boost/system/is_error_code_enum.hpp>
#include <iosfwd>

namespace boost {
namespace socks_proto {

/** Reply code from SOCKS4 server to client

   @par Specification
   @li https://datatracker.ietf.org/doc/html/rfc1928#section-6

 */
enum class reply_code_v4 : uint8_t
{
    /// Request granted
    request_granted                             = 0x00,
    /// General SOCKS server failure
    request_rejected_or_failed                  = 0x01,
    /// Connection not allowed by ruleset
    cannot_connect_to_identd_on_the_client      = 0x02,
    /// Network unreachable
    client_and_identd_report_different_user_ids = 0x03,
    /// Unassigned
    unassigned                                  = 0xFF
};

/** Converts an integer to a known SOCKS4 reply code.

    If the integer does not match a known reply code,
    @ref status::unassigned is returned.
*/
BOOST_SOCKS_PROTO_DECL
reply_code_v4
to_reply_code_v4(unsigned v);

BOOST_SOCKS_PROTO_DECL
error_code
make_error_code(
    reply_code_v4 e) noexcept;

BOOST_SOCKS_PROTO_DECL
error_condition
make_error_condition(
    reply_code_v4 c) noexcept;

/** Returns SOCKS4 reply code as a string view.

    @param v The reply code to use.
*/
BOOST_SOCKS_PROTO_DECL
string_view
to_string(reply_code_v4 v);

/// Outputs the SOCKS4 reply code as a string to a stream.
BOOST_SOCKS_PROTO_DECL
std::ostream&
operator<<(std::ostream&, reply_code_v4);

} // socks_proto
} // boost

namespace boost {
namespace system {

template <>
struct is_error_code_enum<socks_proto::reply_code_v4>
{
    static const bool value = true;
};

} // namespace system
} // namespace boost


#endif
