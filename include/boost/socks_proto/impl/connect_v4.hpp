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

namespace boost {
namespace socks_proto {
namespace detail {

// All these operations are repeatedly
// implementing a pattern that should be
// later encapsulated into
// socks_proto::request
// and socks_proto::reply
template <class Allocator = std::allocator<unsigned char>>
std::vector<unsigned char, Allocator>
prepare_request_v4(
    asio::ip::tcp::endpoint const& target_host,
    core::string_view socks_user,
    Allocator const& a = {})
{
    std::vector<unsigned char, Allocator> buffer(
        9 + socks_user.size(), 0x00, a);

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

BOOST_SOCKS_PROTO_DECL
asio::ip::tcp::endpoint
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
        asio::ip::tcp::endpoint target_host,
        string_view socks_user,
        Allocator const& a)
        : s_(s)
        , buf_(prepare_request_v4(target_host, socks_user, a))
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
            asio::async_write(
                s_,
                asio::buffer(buf_),
                std::move(self));
            // Handle successful CONNECT request
            // Prepare for CONNECT reply
            if (ec.failed())
                goto complete;
            BOOST_ASSERT(buf_.capacity() >= 8);
            buf_.resize(8);
            // Read the CONNECT reply
            BOOST_ASIO_HANDLER_LOCATION((
                __FILE__, __LINE__,
                "asio::async_read"));
            BOOST_ASIO_CORO_YIELD
            asio::async_read(
                s_,
                asio::buffer(buf_),
                std::move(self));
            // Handle successful CONNECT reply
            // Parse the CONNECT reply
            if (ec.failed() &&
                ec != asio::error::eof)
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
            {
                error_code rec;
                ep = parse_reply_v4(
                    buf_.data(),
                    buf_.size(),
                    rec);
                if (rec.failed() &&
                    rec != error::unassigned)
                    ec = rec;
            }
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

} // socks_proto
} // boost



#endif
