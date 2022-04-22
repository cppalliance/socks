//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_IO_DETAIL_CONNECT_V5_HPP
#define BOOST_SOCKS_PROTO_IO_DETAIL_CONNECT_V5_HPP

#include <boost/socks_proto/detail/config.hpp>

#include <boost/socks_proto/io/endpoint.hpp>

#include <boost/socks_proto/string_view.hpp>
#include <boost/socks_proto/error.hpp>
#include <boost/socks_proto/reply_code.hpp>
#include <boost/socks_proto/reply_code_v4.hpp>
#include <boost/socks_proto/version.hpp>
#include <boost/socks_proto/command.hpp>
#include <boost/socks_proto/auth_method.hpp>
#include <boost/socks_proto/address_type.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <boost/core/empty_value.hpp>
#include <boost/core/allocator_access.hpp>

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
prepare_greeting(std::initializer_list<auth_method> il)
{
    BOOST_ASSERT(il.size() <= 255);
    std::vector<unsigned char, Allocator> buffer(
        2 + il.size());

    // VER
    buffer[0] = static_cast<unsigned char>(
        version::socks_5);

    // NMETHODS
    buffer[1] = static_cast<unsigned char>(
        il.size());

    // METHODS
    auto it = il.begin();
    std::size_t i = 2;
    while (it != il.end())
    {
        buffer[i] = static_cast<unsigned char>(*it);
        ++it;
        ++i;
    }

    return buffer;
}

template <class Allocator = std::allocator<unsigned char>>
std::vector<unsigned char, Allocator>
prepare_greeting(io::auth::no_auth const &) {
    return prepare_greeting({auth_method::no_authentication});
}

template <class Allocator = std::allocator<unsigned char>>
std::vector<unsigned char, Allocator>
prepare_greeting(io::auth::userpass const&) {
    return prepare_greeting({auth_method::userpass});
}

inline
std::size_t
dst_addr_size(
    endpoint const& target_host) {
    return target_host.address().is_v6() ? 16 : 4;
}

constexpr
std::size_t
dst_addr_size(
    std::pair<string_view, std::uint16_t> const& target_host) {
    return 1 + target_host.first.size();
}

inline
std::size_t
write_target_host(
    unsigned char* buffer,
    endpoint const& target_host) {
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

inline
std::size_t
write_target_host(
    unsigned char* buffer,
    std::pair<string_view, std::uint16_t> const& target_host) {
    // ATYP
    buffer[0] = static_cast<unsigned char>(
            address_type::domain_name);

    // DST. ADDR
    buffer[1] = static_cast<unsigned char>(
        target_host.first.size());
    std::size_t i = 2;
    for (auto b: target_host.first)
        buffer[i++] = static_cast<unsigned char>(b);

    // DSTPORT
    std::uint16_t target_port = target_host.second;
    buffer[i++] = target_port >> 8;
    buffer[i++] = target_port & 0xFF;

    return i;
}

template <class Endpoint, class Allocator = std::allocator<unsigned char>>
std::vector<unsigned char, Allocator>
prepare_request_v5(
    Endpoint const& target_host,
    Allocator const& a = {})
{
    std::size_t n_dst_addr = dst_addr_size(target_host);
    std::vector<unsigned char, Allocator> buffer(
        6 + n_dst_addr, a);

    // Prepare a CONNECT request
    // VER
    buffer[0] = static_cast<unsigned char>(
        version::socks_5);

    // CMD
    buffer[1] = static_cast<unsigned char>(
        command::connect);

    // RSV
    buffer[2] = 0x00;

    // ATYP + DST. ADDR + DSTPORT
    write_target_host(&buffer[3], target_host);

    return buffer;
}

BOOST_SOCKS_PROTO_DECL
endpoint
parse_reply_v5(
    unsigned char const* buffer,
    std::size_t n,
    error_code& ec);

template <class SyncStream, class EndpointV5, class AuthOptions>
endpoint
connect_v5_any(
    SyncStream& stream,
    EndpointV5&& target_host, // tcp::endpoint or pair<domain, port>
    AuthOptions opt,
    error_code& ec)
{
    // All these functions are repeatedly
    // implementing a pattern that should be
    // encapsulated into socks_proto::request
    // and socks_proto::reply in the future

    // Send a GREETING request
    // Create a list with only opt as a method
    // accepted by the client
    std::vector<unsigned char> buffer =
        detail::prepare_greeting(opt);
    asio::write(
        stream,
        asio::buffer(buffer),
        ec);
    if (ec.failed())
        return endpoint{};

    // Read GREETING reply
    buffer.resize(2);
    std::size_t n = asio::read(
        stream,
        asio::buffer(buffer),
        ec);
    if (ec.failed())
        return endpoint{};

    // AFREITAS Implement sync sub-negotiation

    // Send a CONNECT request
    buffer = detail::prepare_request_v5(target_host);
    asio::write(
        stream,
        asio::buffer(buffer),
        ec);
    if (ec.failed())
        return endpoint{};

    // Read the CONNECT reply
    buffer.resize(22);
    n = asio::read(
        stream,
        asio::buffer(buffer.data(), n),
        ec);
    if (ec.failed() && ec != asio::error::eof)
        return endpoint{};

    buffer.resize(n);
    return detail::parse_reply_v5(
        buffer.data(), buffer.size(), ec);
}

template <class Stream, class Endpoint, class AuthOptions, class Allocator>
class connect_v5_implementation
    : private empty_value<Allocator, 0>
{
public:
    connect_v5_implementation(
        Stream& s,
        Endpoint target_host,
        AuthOptions opt,
        Allocator const& a)
        : empty_value<Allocator, 0>(empty_init, a)
        , s_(s)
        , buf_(prepare_greeting(opt))
        , target_(target_host)
        , opt_(opt)
    {
    }

    template <typename Self>
    void
    operator()(
        Self& self,
        error_code ec = {},
        std::size_t n = 0)
    {
        endpoint ep{};
        BOOST_ASIO_CORO_REENTER(coro_)
        {
            // Send a GREETING request
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_write"));
            BOOST_ASIO_CORO_YIELD
            asio::async_write(
                s_,
                asio::buffer(buf_),
                std::move(self));

            // Read GREETING reply
            if (ec.failed())
                goto complete;
            BOOST_ASSERT(buf_.capacity() >= 2);
            buf_.resize(2);
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_read"));
            BOOST_ASIO_CORO_YIELD
            asio::async_read(
                s_,
                asio::buffer(buf_.data(), buf_.size()),
                std::move(self));

            // AFREITAS Implement sub-negotiation

            // Send the CONNECT request
            if (ec.failed())
                goto complete;
            buf_ =
                prepare_request_v5(target_, this->get());
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_write"));
            BOOST_ASIO_CORO_YIELD
            asio::async_write(
                s_,
                asio::buffer(buf_),
                std::move(self));

            // Read the CONNECT reply
            if (ec.failed())
                goto complete;
            buf_.resize(22);
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_read"));
            BOOST_ASIO_CORO_YIELD
            asio::async_read(
                s_,
                asio::buffer(buf_),
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
            BOOST_ASSERT(buf_.capacity() >= n);
            buf_.resize(n);
            ep = parse_reply_v5(
                buf_.data(), buf_.size(), ec);
        complete:
            {
                // Free memory before invoking the handler
                decltype(buf_) tmp( std::move(buf_) );
            }
            return self.complete(ec, ep);
        }
    }

private:
    Stream& s_;
    std::vector<unsigned char, Allocator> buf_;
    Endpoint target_;
    AuthOptions opt_;
    asio::coroutine coro_;
};

template <class AsyncStream, class Endpoint, class AuthOptions, class CompletionToken>
typename asio::async_result<
    typename asio::decay<CompletionToken>::type,
    void (error_code, endpoint)
    >::return_type
async_connect_v5_any(
    AsyncStream& s,
    Endpoint const& target_host,
    AuthOptions opt,
    CompletionToken&& token)
{
    using DecayedToken =
        typename std::decay<CompletionToken>::type;
    using allocator_type =
        allocator_rebind_t<
            typename asio::associated_allocator<
                DecayedToken>::type, unsigned char>;
    // async_initiate will:
    // - transform token into handler
    // - call initiation_fn(handler, args...)
    return asio::async_compose<
        CompletionToken,
        void (error_code, endpoint)>
        (
            // implementation of the composed asynchronous operation
            detail::connect_v5_implementation<
                AsyncStream, Endpoint, AuthOptions, allocator_type>{
                s,
                target_host,
                opt,
                asio::get_associated_allocator(token)
            },
            // the completion token
            token,
            // I/O objects or I/O executors for which
            // outstanding work must be maintained
            s
        );
}


} // detail
} // io
} // socks_proto
} // boost

#endif
