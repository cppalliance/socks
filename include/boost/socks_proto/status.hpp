//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_STATUS_HPP
#define BOOST_SOCKS_PROTO_STATUS_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <iosfwd>

namespace boost {
namespace socks_proto {

enum class status : uint8_t
{
    request_granted                     = 0x00,
    general_failure                     = 0x01,
    connection_not_allowed_by_ruleset   = 0x02,
    network_unreachable                 = 0x03,
    host_unreachable                    = 0x04,
    connection_refused_by_destination   = 0x05,
    ttl_expired                         = 0x06,
    command_not_supported               = 0x07,
    address_type_not_supported          = 0x08,
    unknown                             = 0xFF,
};

/** Converts the integer to a known status-code.

    If the integer does not match a known status code,
    @ref status::unknown is returned.
*/
BOOST_SOCKS_PROTO_DECL
status
int_to_status(unsigned v);

/** Returns status code as a string.

    @param v The status code to use.
*/
BOOST_SOCKS_PROTO_DECL
string_view
to_string(status v);

/// Outputs the standard reason phrase of a status code to a stream.
BOOST_SOCKS_PROTO_DECL
std::ostream&
operator<<(std::ostream&, status);

} // socks_proto
} // boost

#endif
