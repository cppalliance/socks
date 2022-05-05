//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_IMPL_CONNECT_V4_HPP
#define BOOST_SOCKS_PROTO_IMPL_CONNECT_V4_HPP

#include <boost/socks_proto/detail/config.hpp>

#include <boost/socks_proto/detail/command.hpp>
#include <boost/socks_proto/detail/version.hpp>
#include <boost/socks_proto/error.hpp>
#include <boost/socks_proto/detail/version.hpp>
#include <boost/socks_proto/endpoint.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>

#include <boost/core/allocator_access.hpp>
#include <boost/core/empty_value.hpp>
#include <boost/core/ignore_unused.hpp>

namespace boost {
namespace socks_proto {
namespace detail {

BOOST_SOCKS_PROTO_DECL
std::size_t
prepare_request_v4(
    unsigned char* buffer,
    std::size_t n,
    endpoint const& target_host,
    core::string_view socks_user);

BOOST_SOCKS_PROTO_DECL
endpoint
parse_reply_v4(
    unsigned char const* buffer,
    std::size_t n,
    error_code& ec);

template <class Stream, class Allocator>
class connect_v4_op
{
public:
    connect_v4_op(
        Stream& s,
        endpoint target_host,
        string_view socks_user,
        Allocator const& a)
        : s_(s)
        , buf_(9 + socks_user.size(), 0x00, a)
    {
        std::size_t n = prepare_request_v4(
            buf_.data(),
            buf_.size(),
            target_host,
            socks_user);
        BOOST_ASSERT(n == buf_.size());
        ignore_unused(n);
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
            // Send the CONNECT request
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_write"));
            BOOST_ASIO_CORO_YIELD
            asio::async_write(
                s_,
                asio::buffer(buf_),
                std::move(self));
            if (ec.failed())
                goto complete;

            // Read the CONNECT reply
            BOOST_ASSERT(buf_.capacity() >= 8);
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_read"));
            BOOST_ASIO_CORO_YIELD
            asio::async_read(
                s_,
                asio::buffer(buf_.data(), 8),
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
            if (n != 8)
            {
                ec = error::bad_reply_size;
                goto complete;
            }
            ep = parse_reply_v4(
                buf_.data(), n, ec);
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
    asio::coroutine coro_;
};

} // detail


// These functions are implementing what should
// be encapsulated into socks_proto::request
// in the future.
template <class SyncStream>
endpoint
connect_v4(
    SyncStream& stream,
    endpoint const& target_host,
    string_view socks_user,
    error_code& ec)
{
    // Send a CONNECT request
    // There's no upper bound on the size of
    // a SOCKS4 request
    std::vector<unsigned char> buffer(
        9 + socks_user.size());
    std::size_t n = detail::prepare_request_v4(
        buffer.data(),
        buffer.size(),
        target_host,
        socks_user);
    BOOST_ASSERT(n == buffer.size());
    ignore_unused(n);
    asio::write(
        stream,
        asio::buffer(buffer.data(), n),
        ec);
    if (ec.failed())
        return {};

    // Read the CONNECT reply
    // A CONNECT reply is always 8 bytes
    n = asio::read(
        stream,
        asio::buffer(buffer.data(), 8),
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
    if (n != 8)
    {
        ec = error::bad_reply_size;
        return {};
    }

    // Parse the CONNECT reply
    auto ep = detail::parse_reply_v4(
        buffer.data(), n, ec);
    return ep;
}

// SOCKS4 connect initiating function
// - These overloads look similar to what we
// should have in socks_io.
// - Their implementation includes what should
// be later encapsulated into
// socks_proto::request and socks_proto::reply.
template <class AsyncStream, class CompletionToken>
BOOST_SOCKS_PROTO_ASYNC_ENDPOINT(CompletionToken)
async_connect_v4(
    AsyncStream& s,
    endpoint const& target_host,
    string_view socks_user,
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
        detail::connect_v4_op<
            AsyncStream, allocator_type>{
                s,
                target_host,
                socks_user,
                asio::get_associated_allocator(token)
            },
        // the completion token
        token,
        // I/O objects or I/O executors for which
        // outstanding work must be maintained
        s
    );
}

} // socks_proto
} // boost



#endif
