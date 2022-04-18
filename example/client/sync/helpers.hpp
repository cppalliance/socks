//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef BOOST_SOCKS_PROTO_HELPERS_HPP
#define BOOST_SOCKS_PROTO_HELPERS_HPP

#include <boost/socks_proto/version.hpp>
#include <boost/socks_proto/error.hpp>
#include <boost/socks_proto/reply_code.hpp>
#include <boost/socks_proto/reply_code_v4.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/url/url_view.hpp>
#include <boost/core/detail/string_view.hpp>

std::uint16_t
default_port(const boost::urls::url_view& u)
{
    if (!u.has_port())
    {
        if (u.scheme_id() == boost::urls::scheme::http)
            return 80;
        if (u.scheme_id() == boost::urls::scheme::https)
            return 445;
        if (u.scheme().starts_with("socks"))
            return 1080;
    }
    return u.port_number();
}

// All these operations are repeatedly
// implementing a pattern that should be
// later encapsulated into
// socks_proto::request
// and socks_proto::reply
template <class Allocator = std::allocator<unsigned char>>
std::vector<unsigned char, Allocator>
prepare_request(
    boost::asio::ip::tcp::endpoint const& target_host,
    boost::core::string_view socks_user,
    Allocator const& a = {})
{
    std::vector<unsigned char, Allocator> buffer(
        9 + socks_user.size(), a);

    // Prepare a CONNECT request
    // VER
    buffer[0] = static_cast<unsigned char>(
        boost::socks_proto::version::socks_4);

    // CMD
    buffer[1] = static_cast<unsigned char>(
        boost::socks_proto::command::connect);

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
        buffer[8 + i] = socks_user[i];

    // NULL
    buffer.back() = '\0';

    return buffer;
}

template <class Allocator = std::allocator<unsigned char>>
std::pair<
    boost::system::error_code,
    boost::asio::ip::tcp::endpoint>
parse_reply(std::vector<unsigned char, Allocator> const& buffer)
{
    using error_code = boost::system::error_code;
    namespace asio = boost::asio;
    using tcp = boost::asio::ip::tcp;

    if (buffer.size() < 2)
    {
        // Successful messages have size 8
        // Some servers return only 2 bytes,
        // since DSTPORT and DSTIP can be ignored
        // in SOCKS4 or to return error messages,
        // including SOCKS5 errors
        return {
            asio::error::access_denied,
            tcp::endpoint{} };
    }

    // VER:
    if (buffer[0] == 50)
    {
        // We are trying to connect to a SOCKS5
        // server
        error_code ec =
            boost::socks_proto::to_reply_code(buffer[1]);
        return {ec, tcp::endpoint{}};
    }

    // In SOCKS4, the reply version is allowed to
    // be 0x00. In general, this is the SOCKS version as
    // 40.
    if (buffer[0] != 0x00 && buffer[0] != 40)
    {
        error_code ec =
            boost::socks_proto::to_reply_code_v4(buffer[1]);
        return {ec, tcp::endpoint{}};
    }

    // REP: the res
    auto rep = boost::socks_proto::to_reply_code_v4(buffer[1]);
    if (rep != boost::socks_proto::reply_code_v4::request_granted)
    {
        error_code ec =
            boost::socks_proto::to_reply_code_v4(buffer[1]);
        return {ec, tcp::endpoint{}};
    }

    // DSTPORT and DSTIP might be ignored
    // in SOCKS4, which does not represent
    // an error
    if (buffer.size() < 8)
        return {error_code{}, tcp::endpoint{}};

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
    return {error_code{}, ep};
}

#endif //BOOST_SOCKS_PROTO_HELPERS_HPP
