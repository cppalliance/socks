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

std::size_t
prepare_request_v4(
    unsigned char* buffer,
    std::size_t n,
    asio::ip::tcp::endpoint const& target_host,
    core::string_view socks_user)
{
    BOOST_ASSERT(n >= 9 + socks_user.size());
    ignore_unused(n);

    // Prepare a CONNECT request
    // VER
    buffer[0] = static_cast<unsigned char>(
        version::socks_4);

    // CMD
    buffer[1] = static_cast<unsigned char>(
        command::connect);

    // DSTPORT
    std::uint16_t target_port = target_host.port();
    buffer[2] = target_port >> 8;
    buffer[3] = target_port & 0xFF;

    // DSTIP
    auto ip_bytes =
        target_host.address().to_v4().to_bytes();
    buffer[4] = ip_bytes[0];
    buffer[5] = ip_bytes[1];
    buffer[6] = ip_bytes[2];
    buffer[7] = ip_bytes[3];

    // USERID
    for (std::size_t i = 0; i < socks_user.size(); ++i)
        buffer[8 + i] = static_cast<unsigned char>(
            socks_user[i]);

    // NULL
    buffer[8 + socks_user.size()] = '\0';

    return 9 + socks_user.size();
}


endpoint
parse_reply_v4(
    unsigned char const* buffer,
    std::size_t n,
    error_code& ec)
{
    BOOST_ASSERT(n == 8);

    // VER:
    // In SOCKS4, the *reply* version is allowed to
    // be 0x00. In general, this is the SOCKS version as
    // 0x04.
    if (buffer[0] != 0x00 && buffer[0] != 0x04)
    {
        ec = error::bad_reply_version;
        return {};
    }

    // REP: the res
    ec = static_cast<error>(to_reply_code_v4(buffer[1]));
    if (ec != condition::succeeded)
        return {};

    // DSTPORT
    std::uint16_t port{buffer[2]};
    port = (port << 8) | buffer[3];

    // DSTIP
    std::uint32_t ip{buffer[3]};
    ip = (ip << 8) | buffer[4];
    ip = (ip << 8) | buffer[5];
    ip = (ip << 8) | buffer[6];

    endpoint ep{
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
