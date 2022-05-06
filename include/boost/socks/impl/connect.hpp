//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_IMPL_CONNECT_HPP
#define BOOST_SOCKS_IMPL_CONNECT_HPP

#include <boost/socks/detail/config.hpp>

#include <boost/socks/error.hpp>
#include <boost/socks/detail/auth_method.hpp>
#include <boost/socks/detail/address_type.hpp>
#include <boost/socks/detail/command.hpp>
#include <boost/socks/detail/version.hpp>

#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/core/allocator_access.hpp>
#include <boost/core/empty_value.hpp>

#include <iostream>

namespace boost {
namespace socks {
namespace detail {

struct domain_endpoint_view
{
    string_view domain;
    std::uint16_t port{0};
};

struct domain_endpoint
{
    std::string domain;
    std::uint16_t port{0};

    operator domain_endpoint_view() const
    {
        domain_endpoint_view ep;
        ep.domain = domain;
        ep.port = port;
        return ep;
    }
};

BOOST_SOCKS_DECL
std::size_t
prepare_greeting(
    unsigned char* buffer,
    std::size_t n,
    std::initializer_list<unsigned char> methods);

BOOST_SOCKS_DECL
std::size_t
prepare_greeting(
    unsigned char* buffer,
    std::size_t n,
    auth_options const& opt);

inline
std::size_t
dst_addr_size(
    endpoint const& target_host) {
    return target_host.address().is_v6() ? 16 : 4;
}

constexpr
std::size_t
dst_addr_size(
    domain_endpoint_view const& target_host) {
    return 1 + target_host.domain.size();
}

BOOST_SOCKS_DECL
void
write_ver_cmd_rsv(
    unsigned char* buffer);

BOOST_SOCKS_DECL
std::size_t
write_target_host(
    unsigned char* buffer,
    endpoint const& target_host);

BOOST_SOCKS_DECL
std::size_t
write_target_host(
    unsigned char* buffer,
    domain_endpoint_view const& target_host);

BOOST_SOCKS_DECL
std::size_t
prepare_request(
    unsigned char* buffer,
    std::size_t n,
    endpoint const& target_host);

BOOST_SOCKS_DECL
std::size_t
prepare_request(
    unsigned char* buffer,
    std::size_t n,
    domain_endpoint_view const& target_host);

BOOST_SOCKS_DECL
endpoint
parse_reply_v5(
    unsigned char const* buffer,
    std::size_t n,
    error_code& ec);

BOOST_SOCKS_DECL
void
validate_server_choice(
    unsigned char const* buffer,
    std::size_t n,
    unsigned char code,
    error_code& ec);

BOOST_SOCKS_DECL
void
validate_server_choice(
    unsigned char const* buffer,
    std::size_t n,
    auth_options const& opt,
    error_code& ec);

// authenticate and return server choice
template <class SyncStream>
unsigned char
authenticate(
    SyncStream& stream,
    unsigned char* buffer,
    std::size_t n,
    auth_options const& opt,
    error_code& ec)
{
    // Send a GREETING request
    // Create a list with only opt as a method
    // accepted by the client
    n = prepare_greeting(buffer, n, opt);
    BOOST_ASSERT(n > 2);
    asio::write(
        stream,
        asio::buffer(buffer, n),
        ec);
    if (ec.failed())
        return 0;

    // Read GREETING reply (or "server choice")
    n = asio::read(
        stream,
        asio::buffer(buffer, 2),
        ec);
    if (!ec.failed() ||
        ec == asio::error::eof)
        validate_server_choice(buffer, n, opt, ec);

    // AFREITAS Implement sync sub-negotiation
    // This is ignoring the server methods

    return buffer[1];
}

template <class SyncStream>
std::size_t
write_connect_request(
    SyncStream& stream,
    unsigned char* buffer,
    std::size_t n,
    endpoint const& target_host,
    error_code& ec)
{
    // Send a CONNECT request
    n = detail::prepare_request(
        buffer, n, target_host);
    asio::write(
        stream,
        asio::buffer(buffer, n),
        ec);
    return n;
}

template <class SyncStream>
std::size_t
write_connect_request(
    SyncStream& stream,
    unsigned char* buffer,
    std::size_t n,
    domain_endpoint_view const& target_host,
    error_code& ec)
{
    // Send a CONNECT request
    n = detail::prepare_request(
        buffer, n, target_host);
    asio::write(
        stream,
        asio::buffer(buffer, n),
        ec);
    return n;
}

struct read_reply_cond {
    std::size_t operator()(
        const error_code& ec,
        std::size_t n)
    {
        // max size = 22 because reply should not
        // contain domain name in BND.ADDR
        // min size = 10, when BND.ADDR is ipv4
        if ((n == 10 &&
            to_address_type(buf[3]) == address_type::ip_v4) ||
            ec.failed())
            return 0;
        return 22 - n;
    }

    unsigned char* buf;
};


template <class SyncStream>
endpoint
read_connect_reply(
    SyncStream& stream,
    unsigned char* buffer,
    std::size_t n,
    error_code& ec)
{
    // Read the CONNECT reply
    n = asio::read(
        stream,
        asio::buffer(buffer, 22),
        read_reply_cond{buffer},
        ec);
    if (ec.failed() &&
        ec != asio::error::eof)
    {
        // asio::error::eof indicates there was
        // a SOCKS error and the server
        // closed the connection cleanly.
        // This should happen whenever
        // the reply code is not "succeeded".
        // We still want to parse the response
        // to find out what kind of error.
        return {};
    }
    if (n != 10 &&
        n != 22)
    {
        ec = error::bad_reply_size;
        return {};
    }

    return detail::parse_reply_v5(buffer, n, ec);
}

template <class Stream, class Endpoint, class Allocator>
class connect_op
    : private empty_value<Allocator, 0>
{
public:
    connect_op(
        Stream& s,
        Endpoint target_host,
        auth_options opt,
        Allocator const& a)
        : empty_value<Allocator, 0>(empty_init, a)
        , s_(s)
        , buf_(263, 0x00, a)
        , target_(target_host)
        , opt_(std::move(opt))
    {}

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
            n = prepare_greeting(
                buf_.data(), buf_.size(), opt_);
            BOOST_ASSERT(n > 2);
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_write"));
            BOOST_ASIO_CORO_YIELD
            asio::async_write(
                s_,
                asio::buffer(buf_, n),
                std::move(self));
            if (ec.failed())
                goto complete;

            // Read GREETING reply
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_read"));
            BOOST_ASIO_CORO_YIELD
            asio::async_read(
                s_,
                asio::buffer(buf_.data(), 2),
                std::move(self));
            if (!ec.failed() ||
                ec == asio::error::eof)
                validate_server_choice(
                    buf_.data(), n, opt_, ec);
            if (ec.failed())
                goto complete;

            // AFREITAS Implement sub-negotiation
            // This is ignoring the server method

            // Send the CONNECT request
            n = prepare_request(
                buf_.data(), buf_.size(), target_);
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_write"));
            BOOST_ASIO_CORO_YIELD
            asio::async_write(
                s_,
                asio::buffer(buf_.data(), n),
                std::move(self));
            if (ec.failed())
                goto complete;

            // Read the CONNECT reply
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_read"));
            BOOST_ASIO_CORO_YIELD
            asio::async_read(
                s_,
                asio::buffer(buf_.data(), 22),
                read_reply_cond{buf_.data()},
                std::move(self));
            if (ec.failed() &&
                ec != asio::error::eof)
            {
                // asio::error::eof indicates there was
                // a SOCKS error and the server
                // closed the connection cleanly.
                // This should happen whenever
                // the reply code is not "succeeded".
                // We still want to parse the response
                // to find out what kind of error
                // this is.
                goto complete;
            }
            if (n != 10 &&
                n != 22)
            {
                ec = error::bad_reply_size;
                goto complete;
            }
            ep = parse_reply_v5(buf_.data(), n, ec);
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
    auth_options const opt_;
    asio::coroutine coro_;
};

template <class AsyncStream, class Endpoint, class CompletionToken>
typename asio::async_result<
    typename asio::decay<CompletionToken>::type,
    void (error_code, endpoint)
    >::return_type
async_connect_any(
    AsyncStream& s,
    Endpoint const& target_host,
    auth_options const& opt,
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
            detail::connect_op<
                AsyncStream, Endpoint, allocator_type>{
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


// These functions are implementing what should
// be encapsulated into socks::request
// in the future.
template <class SyncStream>
endpoint
connect(
    SyncStream& stream,
    endpoint const& target_host,
    auth_options const& opt,
    error_code& ec)
{
    unsigned char buffer[263];
    detail::authenticate(
        stream, buffer, 263, opt, ec);
    if (ec.failed())
        return {};

    detail::write_connect_request(
        stream, buffer, 263, target_host, ec);
    if (ec.failed())
        return {};

    auto ep = detail::read_connect_reply(
        stream, buffer, 263, ec);
    return ep;
}

template <class SyncStream>
endpoint
connect(
    SyncStream& stream,
    string_view target_host,
    std::uint16_t target_port,
    auth_options const& opt,
    error_code& ec)
{
    unsigned char buffer[263];
    detail::authenticate(
        stream, buffer, 263, opt, ec);
    if (ec.failed())
        return {};

    detail::domain_endpoint_view target;
    target.domain = target_host;
    target.port = target_port;
    detail::write_connect_request(
        stream, buffer, 263, target, ec);
    if (ec.failed())
        return {};

    auto ep = detail::read_connect_reply(
        stream, buffer, 263, ec);
    if (ec.failed())
        return {};
    return ep;
}

// SOCKS4 connect initiating function
// - These overloads look similar to what we
// should have in socks_io.
// - Their implementation includes what should
// be later encapsulated into
// socks::request and socks::reply.
template <class AsyncStream, class CompletionToken>
BOOST_SOCKS_ASYNC_ENDPOINT(CompletionToken)
async_connect(
    AsyncStream& s,
    endpoint const& target_host,
    auth_options const& opt,
    CompletionToken&& token)
{
    return detail::async_connect_any(
        s, target_host, opt, token);
}

template <class AsyncStream, class CompletionToken>
BOOST_SOCKS_ASYNC_ENDPOINT(CompletionToken)
async_connect(
    AsyncStream& s,
    string_view app_domain,
    std::uint16_t app_port,
    auth_options const& opt,
    CompletionToken&& token)
{
    detail::domain_endpoint ep;
    ep.domain = std::string(app_domain);
    ep.port = app_port;
    return detail::async_connect_any(
        s, ep, opt, token);
}

} // socks
} // boost



#endif
