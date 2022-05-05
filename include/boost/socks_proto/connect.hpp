//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_CONNECT_V5_HPP
#define BOOST_SOCKS_PROTO_CONNECT_V5_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/endpoint.hpp>
#include <boost/socks_proto/auth.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <boost/socks_proto/error.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace boost {
namespace socks_proto {

/** Connect to the application server through a SOCKS5 server

    This function establishes a connection to the
    application server through a SOCKS5 server.

    This composed operation includes the greeting,
    sub-negotiation, and connect steps of the
    SOCKS5 protocol.

    @par Preconditions
    The `SyncStream` should be connected to a
    SOCKS5 server.

    The endpoint might contain IPv4 or IPv6 addresses.

    @par Example
    @code
    boost::asio::connect(s, resolver.resolve(socks_host, socks_service));
    socks_proto::io::auth::no_auth opt{};
    socks_proto::io::connect(s, app_host_endpoint, opt, ec);
    @endcode

    @param s SyncStream connected to a SOCKS server.
    @param ep Application server endpoint.
    @param opt Authentication options.
    @param ec Error code.

    @return server bound address and port

    @par References
    @li <a href="https://www.openssh.com/txt/socks4.protocol">
        SOCKS: A protocol for TCP proxy across firewalls</a>
*/
template <class SyncStream, class AuthOptions>
endpoint
connect(
    SyncStream& s,
    endpoint const& ep,
    AuthOptions opt,
    error_code& ec);

/** Connect to the application server through a SOCKS5 server

    This function establishes a connection to the
    application server through a SOCKS5 server.

    This composed operation includes the greeting,
    sub-negotiation, and connect steps of the
    SOCKS5 protocol.

    The application server is described as the domain
    name of the target host. According to the
    SOCKS5 protocol, this domain name is resolved
    on the SOCKS server.

    @par Preconditions
    The `SyncStream` should be connected to a
    SOCKS5 server.

    The endpoint might contain an IPv4 or IPv5
    address.

    @par Example
    @code
    boost::asio::connect(s, resolver.resolve(socks_host, socks_service));
    socks_proto::io::connect(s, "www.example.com", 80, "username", ec);
    @endcode

    @param s SyncStream connected to a SOCKS server.
    @param app_domain Domain name of the application server
    @param app_port Port of the application server
    @param opt Authentication options
    @param ec Error code

    @par References
    @li <a href="https://www.openssh.com/txt/socks4.protocol">
        SOCKS: A protocol for TCP proxy across firewalls</a>
*/
template <class SyncStream, class AuthOptions>
endpoint
connect(
    SyncStream& s,
    string_view app_domain,
    std::uint16_t app_port,
    AuthOptions opt,
    error_code& ec);

/** Asynchronously connect to the application server through a SOCKS5 server

    This function establishes a connection to the
    application server through a SOCKS5 server.

    This composed operation includes the greeting,
    sub-negotiation, and connect steps of the
    SOCKS5 protocol.

    After this operation, the client can communicate
    with SOCKS5 proxy as if the talking to the
    application server.

    @par Preconditions
    The `AsyncStream` should be connected to a
    SOCKS5 server.

    The endpoint might contain IPv4 or IPv6 address.

    @par Example
    @code
    boost::asio::connect(s, resolver.resolve(socks_host, socks_service));
    socks_io::async_connect(
        s, app_host_endpoint, "username", ec,
        [](error_code ec, endpoint ep)
    {
        if (!ec.failed())
        {
            // write to the application host
            do_write();
        }
    });
    @endcode

    @param s SyncStream connected to a SOCKS server.
    @param ep Application server endpoint.
    @param opt Authentication options
    @param token Asio CompletionToken.

    @par References
    @li <a href="https://www.openssh.com/txt/socks4.protocol">
        SOCKS: A protocol for TCP proxy across firewalls</a>

*/
template <class AsyncStream, class CompletionToken>
BOOST_SOCKS_PROTO_ASYNC_ENDPOINT(CompletionToken)
async_connect(
    AsyncStream& s,
    endpoint const& ep,
    AuthOptions opt,
    CompletionToken&& token);

/** Asynchronously connect to the application server through a SOCKS5 server

    This function establishes a connection to the
    application server through a SOCKS5 server.

    This composed operation includes the greeting,
    sub-negotiation, and connect steps of the
    SOCKS5 protocol.

    The application server is described as the domain
    name of the target host. According to the
    SOCKS5 protocol, this domain name is resolved
    on the SOCKS server.

    After this operation, the client can communicate
    with SOCKS5 proxy as if the talking to the
    application server.

    @par Preconditions
    The `AsyncStream` should be connected to a
    SOCKS5 server.

    The endpoint might contain IPv4 or IPv6 address.

    @par Example
    @code
    boost::asio::connect(s, resolver.resolve(socks_host, socks_service));
    socks_io::async_connect(
        s, "www.example.com", 80, "username", ec,
        [](error_code ec, endpoint ep)
    {
        if (!ec.failed())
        {
            // write to the application host
            do_write();
        }
    });
    @endcode

    @param s SyncStream connected to a SOCKS server.
    @param app_domain Domain name of the application server
    @param app_port Port of the application server
    @param opt Authentication options
    @param token Asio CompletionToken.

    @par References
    @li <a href="https://www.openssh.com/txt/socks4.protocol">
        SOCKS: A protocol for TCP proxy across firewalls</a>

*/
template <class AsyncStream, class CompletionToken>
BOOST_SOCKS_PROTO_ASYNC_ENDPOINT(CompletionToken)
async_connect(
    AsyncStream& s,
    string_view app_domain,
    std::uint16_t app_port,
    AuthOptions opt,
    CompletionToken&& token);

} // socks_proto
} // boost

#include <boost/socks_proto/impl/connect.hpp>

#endif
