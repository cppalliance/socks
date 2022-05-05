//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_SOCKS_PROTO_CONNECT_V4_HPP
#define BOOST_SOCKS_PROTO_CONNECT_V4_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/endpoint.hpp>
#include <boost/socks_proto/string_view.hpp>
#include <boost/socks_proto/error.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace boost {
namespace socks_proto {

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

        SOCKS: A protocol for TCP proxy across firewalls</a>
*/
template <class SyncStream>
endpoint
connect_v4(
    SyncStream& s,
    endpoint const& ep,
    string_view ident_id,
    error_code& ec);

/** Asynchronously connect to the application server through a SOCKS4 server


    After this operation, the client can communicate

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
    @param ident_id Client ident ID.
    @param token Completion token.

    @return server bound address and port

    @par References
    @li <a href="https://www.openssh.com/txt/socks4.protocol">
        SOCKS: A protocol for TCP proxy across firewalls</a>

*/
template <class AsyncStream, class CompletionToken>
BOOST_SOCKS_PROTO_ASYNC_ENDPOINT(CompletionToken)
async_connect_v4(
    AsyncStream& s,
    endpoint const& ep,
    string_view ident_id,
    CompletionToken&& token);

} // socks_proto
} // boost

#include <boost/socks_proto/impl/connect_v4.hpp>

#endif
