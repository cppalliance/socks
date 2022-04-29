//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_IO_IMPL_CONNECT_HPP
#define BOOST_SOCKS_PROTO_IO_IMPL_CONNECT_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/core/allocator_access.hpp>
#include <boost/socks_proto/io/detail/connect.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <utility>

namespace boost {
namespace socks_proto {
namespace io {

// These functions are implementing what should
// be encapsulated into socks_proto::request
// in the future.
template <class SyncStream, class AuthOptions>
endpoint
connect(
    SyncStream& stream,
    endpoint const& target_host,
    AuthOptions opt,
    error_code& ec)
{
    return detail::connect_any(
        stream, target_host, opt, ec);
}

template <class SyncStream, class AuthOptions>
endpoint
connect(
    SyncStream& stream,
    string_view target_host,
    std::uint16_t target_port,
    AuthOptions opt,
    error_code& ec)
{
    return detail::connect_any(
        stream,
        std::make_pair(target_host, target_port),
        opt,
        ec);
}

// SOCKS4 connect initiating function
// - These overloads look similar to what we
// should have in socks_io.
// - Their implementation includes what should
// be later encapsulated into
// socks_proto::request and socks_proto::reply.
template <class AsyncStream, class AuthOptions, class CompletionToken>
typename asio::async_result<
    typename asio::decay<CompletionToken>::type,
    void (error_code, endpoint)
>::return_type
async_connect(
    AsyncStream& s,
    endpoint const& target_host,
    AuthOptions opt,
    CompletionToken&& token)
{
    return detail::async_connect_any(
        s, target_host, opt, token);
}

template <class AsyncStream, class AuthOptions, class CompletionToken>
typename asio::async_result<
    typename asio::decay<CompletionToken>::type,
    void (error_code, endpoint)
    >::return_type
async_connect(
    AsyncStream& s,
    string_view app_domain,
    std::uint16_t app_port,
    AuthOptions opt,
    CompletionToken&& token)
{
    return detail::async_connect_any(
        s, std::make_pair(std::string(app_domain), app_port), opt, token);
}

} // io
} // socks_proto
} // boost



#endif
