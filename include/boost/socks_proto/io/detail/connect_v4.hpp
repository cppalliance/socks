//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_IO_DETAIL_CONNECT_V4_HPP
#define BOOST_SOCKS_PROTO_IO_DETAIL_CONNECT_V4_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <boost/socks_proto/error.hpp>
#include <boost/socks_proto/reply_code.hpp>
#include <boost/socks_proto/reply_code_v4.hpp>
#include <boost/socks_proto/version.hpp>
#include <boost/socks_proto/command.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <boost/core/empty_value.hpp>

namespace boost {
namespace socks_proto {
namespace io {
namespace detail {

// All these operations are repeatedly
// implementing a pattern that should be
// later encapsulated into
// socks_proto::request
// and socks_proto::reply
template <class Allocator = std::allocator<unsigned char>>
std::vector<unsigned char, Allocator>
prepare_request_v4(
    boost::asio::ip::tcp::endpoint const& target_host,
    boost::core::string_view socks_user,
    Allocator const& a = {})
{
    std::vector<unsigned char, Allocator> buffer(
        9 + socks_user.size(), a);

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
        buffer[8 + i] = socks_user[i];

    // NULL
    buffer.back() = '\0';

    return buffer;
}

template <class Allocator = std::allocator<unsigned char>>
std::pair<
    boost::system::error_code,
    boost::asio::ip::tcp::endpoint>
parse_reply_v4(std::vector<unsigned char, Allocator> const& buffer)
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

template <class Stream, class Allocator>
class connect_v4_implementation
{
public:
    connect_v4_implementation(
        Stream& s,
        asio::ip::tcp::endpoint target_host,
        string_view socks_user,
        Allocator const& a)
        : stream_(s)
        , buffer_(prepare_request_v4(target_host, socks_user, a))
    {
    }

    template <typename Self>
    void
    operator()(
        Self& self,
        error_code ec = {},
        std::size_t n = 0)
    {
        asio::ip::tcp::endpoint ep{};
        BOOST_ASIO_CORO_REENTER(coro_)
        {
            // Send the CONNECT request
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_write"));
            BOOST_ASIO_CORO_YIELD
            boost::asio::async_write(
                stream_,
                asio::buffer(buffer_),
                std::move(self));
            // Handle successful CONNECT request
            // Prepare for CONNECT reply
            if (ec.failed())
                goto complete;
            BOOST_ASSERT(buffer_.capacity() >= 8);
            buffer_.resize(8);
            // Read the CONNECT reply
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_read"));
            BOOST_ASIO_CORO_YIELD
            asio::async_read(
                stream_,
                asio::buffer(buffer_),
                std::move(self)
            );
            // Handle successful CONNECT reply
            // Parse the CONNECT reply
            if (ec.failed() && ec != asio::error::eof)
            {
                // asio::error::eof indicates there was
                // a SOCKS error and the server
                // closed the connection cleanly
                // we still want to parse the response
                // to find out what kind of error
                goto complete;
            }
            BOOST_ASSERT(buffer_.capacity() >= n);
            buffer_.resize(n);
            {
                error_code rec;
                std::tie(rec, ep) = parse_reply_v4(buffer_);
                if (rec.failed() &&
                    rec != socks_proto::reply_code_v4::unassigned)
                    ec = rec;
            }
        complete:
            {
                // Free memory before invoking the handler
                decltype(buffer_) tmp( std::move(buffer_) );
            }
            return self.complete(ec, ep);
        }
    }

private:
    Stream& stream_;
    std::vector<unsigned char, Allocator> buffer_;
    boost::asio::coroutine coro_;
};

template <class Stream, class Allocator>
class resolve_and_connect_v4_implementation
    : private boost::empty_value<Allocator, 0>
{
public:
    resolve_and_connect_v4_implementation(
        Stream& s,
        string_view app_domain,
        std::uint16_t app_port,
        string_view socks_user,
        Allocator const& a)
        : boost::empty_value<Allocator, 0>(boost::empty_init, a)
        , stream_(s)
        , app_domain_(app_domain, allocator_rebind_t<Allocator, char>(a))
        , app_port_(std::to_string(app_port), allocator_rebind_t<Allocator, char>(a))
        , socks_user_(socks_user, allocator_rebind_t<Allocator, char>(a))
    {
    }

    template <typename Self>
    void
    operator()(Self& self)
    {
        state_ = resolving;
        asio::ip::tcp::resolver resolver(
            self.get_executor());
        BOOST_ASIO_HANDLER_LOCATION((
            __FILE__, __LINE__,
            "resolver::async_resolve"));
        resolver.async_resolve(
            app_domain_,
            app_port_,
            std::move(self));
    }

    template <typename Self>
    void
    operator()(Self& self,
               error_code ec,
               asio::ip::tcp::resolver::results_type endpoints)
    {
        if (ec.failed())
            return complete_now(self, ec, asio::ip::tcp::endpoint{});

        // Filter the IPv4 address
        state_ = connecting;
        auto it = endpoints.begin();
        while (it != endpoints.end())
        {
            auto e = it->endpoint();
            if (e.address().is_v4())
            {
                // Send the CONNECT request
                BOOST_ASIO_HANDLER_LOCATION((
                    __FILE__, __LINE__,
                    "socks_proto::connect_v4"));
                async_connect_v4(
                    stream_,
                    e,
                    socks_user_,
                    std::move(self)
                );
                return;
            }
            ++it;
        }

        return complete_now(
            self,
            asio::error::host_not_found,
            asio::ip::tcp::endpoint{});
    }

    template <typename Self>
    void
    operator()(Self& self,
               error_code ec,
               asio::ip::tcp::endpoint ep)
    {
        return complete_now(self, ec, ep);
    }

    template <typename Self>
    void
    complete_now(Self& self, error_code ec, asio::ip::tcp::endpoint ep)
    {
        {
            // Free memory before invoking the handler
            decltype(app_domain_) tmp1( std::move(app_domain_) );
            decltype(app_port_) tmp2( std::move(app_port_) );
            decltype(socks_user_) tmp3( std::move(socks_user_) );
        }
        return self.complete(ec, ep);
    }

private:
    Stream& stream_;
    using io_string_type =
        std::basic_string<
            char,
            std::char_traits<char>,
            allocator_rebind_t<Allocator, char>
        >;
    io_string_type app_domain_;
    io_string_type app_port_;
    io_string_type socks_user_;
    enum { starting, resolving, connecting } state_{starting};

};

} // detail
} // io
} // socks_proto
} // boost

#endif
