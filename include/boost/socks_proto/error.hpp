//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_ERROR_HPP
#define BOOST_SOCKS_PROTO_ERROR_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/result.hpp>

namespace boost {
namespace socks_proto {

/// The type of error code used by the library
using error_code = system::error_code;

/// The type of system error thrown by the library
using system_error = system::system_error;

/// The type of error category used by the library
using error_category = system::error_category;

/// The type of error condition used by the library
using error_condition = system::error_condition;

/// The type of result returned by library functions
template<class T>
using result = boost::system::result<T, error_code>;


/** Error codes returned by SOCKS operations
*/
enum class error
{
    //
    // SOCKS5 reply errors (0x00 to 0x08)
    //

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

    //----------------------------------

    //
    // SOCKS4 reply errors (0x5A to 0x5D)
    //

    /// Request granted
    request_granted                             = 90,

    /// Request rejected or failed
    request_rejected_or_failed                  = 91,

    /// Request rejected because SOCKS server cannot connect to identd on the client
    cannot_connect_to_identd_on_the_client      = 92,

    /// Request rejected because the client program and identd report different user-ids
    client_and_identd_report_different_user_ids = 93,

    //----------------------------------

    //
    // Client errors
    //

    /// Bad reply size
    bad_reply_size              = 0xA0,

    /// Bad reply version
    bad_reply_version,

    /// Bad authentication server choice
    bad_auth_server_choice,

    /// Bad reply command
    bad_reply_command,

    /// Bad reserved component
    bad_reserved_component,

    /// Bad address type
    bad_address_type,


    //----------------------------------

    // Enum reserved for unassigned errors,
    // which can be:
    // - SOCKS5 (0x09-0xFF)
    // - SOCKS4 (0x00-0x59 and 0x5E-0xFF)

    /// Unassigned
    unassigned                          = 0xFF,
};

/** Error conditions corresponding to SOCKS errors
*/
enum class condition
{
    /// SOCKS reply successful
    succeeded,

    /// SOCKS reply with error
    reply_error,

    /// SOCKS proxy error
    proxy_error,

    /// Cannot parse a request or reply
    parse_error,
};

} // socks_proto
} // boost

#include <boost/socks_proto/impl/error.hpp>

#endif
