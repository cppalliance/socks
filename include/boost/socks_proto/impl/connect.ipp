//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef BOOST_SOCKS_PROTO_IMPL_CONNECT_IPP
#define BOOST_SOCKS_PROTO_IMPL_CONNECT_IPP

#include <boost/socks_proto/connect.hpp>
#include <boost/socks_proto/detail/reply_code.hpp>

namespace boost {
namespace socks_proto {
namespace detail {

std::size_t
prepare_greeting(
    unsigned char* buffer,
    std::size_t n,
    std::initializer_list<unsigned char> methods)
{
    BOOST_ASSERT(methods.size() <= 255);
    if (n < 2 + methods.size())
        return 0;

    // VER
    buffer[0] = static_cast<unsigned char>(
        version::socks_5);

    // NMETHODS
    buffer[1] = static_cast<unsigned char>(
        methods.size());

    // METHODS
    auto it = methods.begin();
    std::size_t i = 2;
    while (it != methods.end())
    {
        buffer[i] = static_cast<unsigned char>(*it);
        ++it;
        ++i;
    }

    return i;
}

std::size_t
prepare_greeting(
    unsigned char* buffer,
    std::size_t n,
    auth_options const& opt)
{
    unsigned char auth_code = opt.code();
    if (auth_code != 0x00)
        return prepare_greeting(
            buffer, n, {0x00, auth_code});
    else
        return prepare_greeting(buffer, n, {0x00});
}

void
write_ver_cmd_rsv(
    unsigned char* buffer)
{
    // Prepare a CONNECT request
    // VER
    buffer[0] = static_cast<unsigned char>(
        version::socks_5);

    // CMD
    buffer[1] = static_cast<unsigned char>(
        command::connect);

    // RSV
    buffer[2] = 0x00;
}

std::size_t
write_target_host(
    unsigned char* buffer,
    endpoint const& target_host)
{
    // ATYP
    buffer[0] = static_cast<unsigned char>(
        target_host.address().is_v6() ?
            address_type::ip_v6 : address_type::ip_v4);

    // DST. ADDR
    std::size_t i = 1;
    if (target_host.address().is_v4())
    {
        auto ip_bytes =
            target_host.address().to_v4().to_bytes();
        for (auto b: ip_bytes)
            buffer[i++] = b;
    }
    else
    {
        auto ip_bytes =
            target_host.address().to_v6().to_bytes();
        for (auto b: ip_bytes)
            buffer[i++] = b;
    }

    // DSTPORT
    std::uint16_t target_port = target_host.port();
    buffer[i++] = target_port >> 8;
    buffer[i++] = target_port & 0xFF;

    return i;
}

std::size_t
write_target_host(
    unsigned char* buffer,
    domain_endpoint_view const& target_host)
{
    // ATYP
    buffer[0] = static_cast<unsigned char>(
            address_type::domain_name);

    // DST. ADDR
    buffer[1] = static_cast<unsigned char>(
        target_host.domain.size());
    std::size_t i = 2;
    for (auto b: target_host.domain)
        buffer[i++] = static_cast<unsigned char>(b);

    // DSTPORT
    std::uint16_t target_port = target_host.port;
    buffer[i++] = target_port >> 8;
    buffer[i++] = target_port & 0xFF;

    return i;
}

std::size_t
prepare_request(
    unsigned char* buffer,
    std::size_t n,
    endpoint const& target_host)
{
    std::size_t n_dst_addr = dst_addr_size(target_host);
    BOOST_ASSERT(n >= 6 + n_dst_addr);

    // VER + CMD + RSV
    write_ver_cmd_rsv(buffer);

    // ATYP + DST. ADDR + DSTPORT
    write_target_host(&buffer[3], target_host);

    return n_dst_addr;
}

std::size_t
prepare_request(
    unsigned char* buffer,
    std::size_t n,
    domain_endpoint_view const& target_host)
{
    std::size_t n_dst_addr = dst_addr_size(target_host);
    BOOST_ASSERT(n >= 6 + n_dst_addr);

    // VER + CMD + RSV
    write_ver_cmd_rsv(buffer);

    // ATYP + DST. ADDR + DSTPORT
    write_target_host(&buffer[3], target_host);

    return n_dst_addr;
}

endpoint
parse_reply_v5(
    unsigned char const* buffer,
    std::size_t n,
    error_code& ec)
{
    BOOST_ASSERT(n == 10 || n == 22);

    // VER:
    // In SOCKS5, the reply version is allowed to
    // be 0x00. In general, this is the SOCKS version as
    // 40.
    if (buffer[0] != 0x05)
    {
        ec = error::bad_reply_version;
        return {};
    }

    // REP: the res
    ec = static_cast<error>(to_reply_code(buffer[1]));
    if (ec != condition::succeeded)
        return {};

    // ATYP
    address_type atyp = to_address_type(buffer[3]);

    // DSTIP
    switch (atyp)
    {
    case address_type::ip_v4:
    {
        if (n < 10)
        {
            return {};
        }
        std::uint32_t ip{ buffer[4] };
        ip = (ip << 8) | buffer[5];
        ip = (ip << 8) | buffer[6];
        ip = (ip << 8) | buffer[7];
        std::uint16_t port{ buffer[8] };
        port <<= 8;
        port |= buffer[9];
        return endpoint{
            asio::ip::make_address_v4(ip),
            port
        };
    }
    case address_type::ip_v6:
    {
        if (n < 22)
        {
            return {};
        }
        asio::ip::address_v6::bytes_type ip;
        std::size_t i = 0;
        std::memcpy(ip.data(), buffer + 4, 16);
        std::uint16_t port{ buffer[i++] };
        port <<= 8;
        port |= buffer[i++];
        return endpoint{
            asio::ip::make_address_v6(ip),
            port
        };
    }
    default:
        ec = error::general_failure;
        return {};
    }
}

void
validate_server_choice(
    unsigned char const* buffer,
    std::size_t n,
    unsigned char code,
    error_code& ec)
{
    if (ec.failed())
        return;
    else if (n < 2)
        ec = error::bad_reply_size;
    else if (buffer[0] != 0x05)
        ec = error::bad_reply_version;
    else if (buffer[1] != code &&
             buffer[1] != 0x00)
        ec = error::bad_auth_server_choice;
}

void
validate_server_choice(
    unsigned char const* buffer,
    std::size_t n,
    auth_options const& opt,
    error_code& ec)
{
    validate_server_choice(buffer, n, opt.code(), ec);
}

} // detail
} // socks_proto
} // boost

#endif
