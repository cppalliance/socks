//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_IO_CONNECT_V4_HPP
#define BOOST_SOCKS_PROTO_IO_CONNECT_V4_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <boost/socks_proto/error.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace boost {
namespace socks_proto {
namespace io {

/** Connect to the application server through a SOCKS4 server

    This function establishes a connection to the
    application server through a SOCKS4 server.

    @par Preconditions
    The `SyncStream` should be connected to a
    SOCKS4 server.

    The endpoint must contain an IPv4 address.

    @par Example
    @code
    boost::asio::connect(s, resolver.resolve(socks_host, socks_service));
    socks_proto::io::connect_v4(s, app_host_endpoint, "username", ec);
    @endcode

    @param s SyncStream connected to a SOCKS server.
    @param ep Application server endpoint.
    @param ident_id SOCKS client user id.
    @param ec Error code.

    @par References
    @li <a href="https://www.openssh.com/txt/socks4.protocol">
        SOCKS: A protocol for TCP proxy across firewalls</a>
*/
template <class SyncStream>
asio::ip::tcp::endpoint
connect_v4(
    SyncStream& s,
    asio::ip::tcp::endpoint const& ep,
    string_view ident_id,
    error_code& ec);

/** Connect to the application server through a SOCKS4 server

    This function establishes a connection to the
    application server through a SOCKS4 server.

    The application server is described as the domain
    name of the target host. According to the
    SOCKS4 protocol, this domain name is resolved
    on the client.

    @par Preconditions
    The `SyncStream` should be connected to a
    SOCKS4 server.

    The application server domain name should
    be resolvable to an IPv4 address.

    @par Example
    @code
    boost::asio::connect(s, resolver.resolve(socks_host, socks_service));
    socks_proto::io::connect_v4(s, "www.example.com", 80, "username", ec);
    @endcode

    @param s SyncStream connected to a SOCKS server.
    @param app_domain Domain name of the application server
    @param app_port Port of the application server
    @param ident_id SOCKS client user id.
    @param ec Error code

    @par References
    @li <a href="https://www.openssh.com/txt/socks4.protocol">
        SOCKS: A protocol for TCP proxy across firewalls</a>
*/
template <class SyncStream>
asio::ip::tcp::endpoint
connect_v4(
    SyncStream& s,
    string_view app_domain,
    std::uint16_t app_port,
    string_view ident_id,
    error_code& ec);

/** Asynchronously connect to the application server through a SOCKS4 server

    This function establishes a connection to the
    application server through a SOCKS4 server.

    After this operation, the client can communicate
    with SOCKS4 proxy as if the talking to the
    application server.

    All SOCKS4 CONNECT requests contain a `ident_id`
    to identify the user, where a connection
    without authentication can be represented with
    an empty id.

    @par Preconditions
    The `AsyncStream` should be connected to a
    SOCKS4 server.

    The endpoint must contain an IPv4 address.

    @par Example
    @code
    boost::asio::connect(s, resolver.resolve(socks_host, socks_service));
    socks_io::async_connect_v4(
        s, app_host_endpoint, "username", ec,
        [](error_code ec, asio::ip::tcp::endpoint ep)
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
    @param ident_id SOCKS client user id.
    @param token Asio CompletionToken.

    @par References
    @li <a href="https://www.openssh.com/txt/socks4.protocol">
        SOCKS: A protocol for TCP proxy across firewalls</a>

*/
template <class AsyncStream, class CompletionToken>
typename asio::async_result<
    typename asio::decay<CompletionToken>::type,
    void (error_code, asio::ip::tcp::endpoint)
>::return_type
async_connect_v4(
    AsyncStream& s,
    asio::ip::tcp::endpoint const& ep,
    string_view ident_id,
    CompletionToken&& token);

/** Asynchronously connect to the application server through a SOCKS4 server

    This function establishes a connection to the
    application server through a SOCKS4 server.



    After this operation, the client can communicate
    with SOCKS4 proxy as if the talking to the
    application server.

    All SOCKS4 CONNECT requests contain a `ident_id`
    to identify the user, where a connection
    without authentication can be represented with
    an empty id.

    @par Preconditions
    The `AsyncStream` should be connected to a
    SOCKS4 server.

    The endpoint must contain an IPv4 address.

    @par Example
    @code
    boost::asio::connect(s, resolver.resolve(socks_host, socks_service));
    socks_io::async_connect_v4(
        s, app_host_endpoint, "username", ec,
        [](error_code ec, asio::ip::tcp::endpoint ep)
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
    @param ident_id SOCKS client user id.
    @param token Asio CompletionToken.

    @return server bound address and port

    @par References
    @li <a href="https://www.openssh.com/txt/socks4.protocol">
        SOCKS: A protocol for TCP proxy across firewalls</a>

*/
template <class AsyncStream, class CompletionToken>
typename asio::async_result<
    typename asio::decay<CompletionToken>::type,
    void (error_code, asio::ip::tcp::endpoint)
>::return_type
async_connect_v4(
    AsyncStream& s,
    string_view app_domain,
    std::uint16_t app_port,
    string_view ident_id,
    CompletionToken&& token);

} // io
} // socks_proto
} // boost

#include <boost/socks_proto/io/impl/connect_v4.hpp>

#endif
