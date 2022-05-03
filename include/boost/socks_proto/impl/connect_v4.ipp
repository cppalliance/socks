//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef BOOST_SOCKS_PROTO_IMPL_CONNECT_V4_IPP
#define BOOST_SOCKS_PROTO_IMPL_CONNECT_V4_IPP

#include <boost/socks_proto/connect_v4.hpp>
#include <boost/socks_proto/detail/reply_code_v4.hpp>
#include <boost/core/ignore_unused.hpp>

namespace boost {
namespace socks_proto {
namespace detail {

asio::ip::tcp::endpoint
parse_reply_v4(
    unsigned char const* buffer,
    std::size_t n,
    error_code& ec)
{
    namespace asio = asio;
    using tcp = asio::ip::tcp;
    ec = {};

    if (n < 2)
    {
        // Successful messages have size 8
        // Some servers return only 2 bytes,
        // since DSTPORT and DSTIP can be ignored
        // in SOCKS4 or to return error messages,
        // including SOCKS5 errors
        ec = asio::error::access_denied;
        return {};
    }

    // VER:
    // In SOCKS4, the reply version is allowed to
    // be 0x00. In general, this is the SOCKS version as
    // 0x04.
    if (buffer[0] != 0x00 && buffer[0] != 0x04)
    {
        ec = asio::error::no_protocol_option;
        return {};
    }

    // REP: the res
    auto rep = to_reply_code_v4(buffer[1]);
    if (rep != reply_code_v4::request_granted)
    {
        ec = static_cast<error>(buffer[1]);
        return {};
    }

    // DSTPORT and DSTIP might be ignored
    // in SOCKS4, which does not represent
    // an error
    if (n < 8)
    {
        ec = {};
        return {};
    }

    // DSTPORT
    std::uint16_t port{buffer[2]};
    port = (port << 8) | buffer[3];

    // DSTIP
    std::uint32_t ip{buffer[3]};
    ip = (ip << 8) | buffer[4];
    ip = (ip << 8) | buffer[5];
    ip = (ip << 8) | buffer[6];

    tcp::endpoint ep{
        asio::ip::make_address_v4(ip),
        port
    };
    ec = {};
    return ep;
}

} // detail
} // socks_proto
} // boost

#endif
