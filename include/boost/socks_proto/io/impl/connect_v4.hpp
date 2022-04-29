//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_IO_IMPL_CONNECT_V4_HPP
#define BOOST_SOCKS_PROTO_IO_IMPL_CONNECT_V4_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/io/endpoint.hpp>
#include <boost/socks_proto/io/detail/connect_v4.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/core/allocator_access.hpp>

namespace boost {
namespace socks_proto {
namespace io {

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
    // All these functions are repeatedly
    // implementing a pattern that should be
    // encapsulated into socks_proto::request
    // and socks_proto::reply in the future
    std::vector<unsigned char> buffer =
        detail::prepare_request_v4(
            target_host, socks_user);

    // Send a CONNECT request
    asio::write(
        stream,
        asio::buffer(buffer),
        ec);
    if (ec.failed())
        return endpoint{};

    // Read the CONNECT reply
    buffer.resize(8);
    std::size_t n = asio::read(
        stream,
        asio::buffer(buffer.data(), buffer.size()),
        ec);
    if (ec.failed() && ec != asio::error::eof)
        return endpoint{};

    // Parse the CONNECT reply
    buffer.resize(n);
    return detail::parse_reply_v4(
        buffer.data(), buffer.size(), ec);
}

// SOCKS4 connect initiating function
// - These overloads look similar to what we
// should have in socks_io.
// - Their implementation includes what should
// be later encapsulated into
// socks_proto::request and socks_proto::reply.
template <class AsyncStream, class CompletionToken>
typename asio::async_result<
    typename asio::decay<CompletionToken>::type,
    void (error_code, endpoint)
>::return_type
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

} // io
} // socks_proto
} // boost



#endif
