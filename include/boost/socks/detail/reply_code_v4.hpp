//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_DETAIL_REPLY_CODE_V4_HPP
#define BOOST_SOCKS_DETAIL_REPLY_CODE_V4_HPP

#include <boost/socks/detail/config.hpp>
#include <boost/socks/error.hpp>
#include <boost/socks/string_view.hpp>
#include <boost/system/is_error_code_enum.hpp>
#include <iosfwd>

namespace boost {
namespace socks {
namespace detail {

// Reply code from SOCKS4 server to client
//
// https://datatracker.ietf.org/doc/html/rfc1928#section-6
enum class reply_code_v4 : uint8_t
{
    // Request granted
    request_granted                             = 90,

    // General SOCKS server failure
    request_rejected_or_failed                  = 91,

    // Connection not allowed by ruleset
    cannot_connect_to_identd_on_the_client      = 92,

    // Network unreachable
    client_and_identd_report_different_user_ids = 93,

    // Unassigned
    unassigned                                  = 0xFF
};

// Converts an integer to a known SOCKS4 reply code.
//
//  If the integer does not match a known reply code,
//  reply_code_v4::unassigned is returned.
BOOST_SOCKS_DECL
reply_code_v4
to_reply_code_v4(unsigned v);

} // detail
} // socks
} // boost

#endif
