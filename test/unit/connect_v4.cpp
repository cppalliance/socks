//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

// Test that header file is self-contained.
#include <boost/socks/connect_v4.hpp>
#include <boost/socks/detail/reply_code_v4.hpp>
#include "test_suite.hpp"
#include "stream.hpp"
#include <array>

namespace boost {
namespace socks {

class io_connect_v4_test
{
public:
    using io_context = asio::io_context;
    using endpoint = asio::ip::tcp::endpoint;
    using reply_code_v4 = detail::reply_code_v4;

    static
    std::vector<unsigned char>
    make_request(
        endpoint const& ep,
        string_view user)
    {
        std::vector<unsigned char> r(9 + user.size());
        std::size_t n =
            detail::prepare_request_v4(
                r.data(), r.size(), ep, user);
        BOOST_ASSERT(r.size() == n);
        ignore_unused(n);
        return r;
    }

    static
    std::array<unsigned char, 8>
    make_reply(
        reply_code_v4 r = reply_code_v4::request_granted)
    {
        return {{
            0x04, // VER
            static_cast<unsigned char>(r), // REP
            0x00, // DSTPORT
            0x00,
            0x00, // DSTIP
            0x00,
            0x00,
            0x00
        }};
    }

    template <
        class ConstBufferSequence,
        typename std::enable_if<
            asio::is_const_buffer_sequence<
                ConstBufferSequence
                >::value,
            int>::type = 0
        >
    static
    void
    checkEndpoint(
        ConstBufferSequence const& reply,
        string_view user,
        error_code exp_ec,
        std::size_t fail_at = 0,
        error_code fail_with = {},
        endpoint const& ep = {
            asio::ip::make_address_v4(
                asio::ip::address_v4::uint_type(0)),
                0})
    {
        io_context ioc;
        test::stream s(ioc, fail_at, fail_with);
        s.reset_read(reply);
        error_code ec;
        endpoint app_ep = connect_v4(s, ep, user, ec);
        auto buf = make_request(ep, user);
        BOOST_TEST(s.equal_write_buffers(asio::buffer(buf)));
        BOOST_TEST_EQ(app_ep.address().to_v4(),
                      asio::ip::make_address_v4("0.0.0.0"));
        BOOST_TEST_EQ(ec, exp_ec);
    };

    template <std::size_t N>
    static
    void
    checkEndpoint(
        std::array<unsigned char, N> const& reply,
        string_view user,
        error_code exp_ec,
        std::size_t fail_at = 0,
        error_code fail_with = {},
        endpoint const& ep = {
            asio::ip::make_address_v4(
                asio::ip::address_v4::uint_type(0)),
                0})
    {
        checkEndpoint(
            asio::buffer(reply),
            user,
            exp_ec,
            fail_at,
            fail_with,
            ep);
    }

    static
    void
    testEndpoint()
    {
        // no auth
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_reply(),
                "",
                error::request_granted);
        }

        // user
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_reply(),
                "username",
                error::request_granted);
        }

        // reply buf too small
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                std::array<unsigned char, 1>{{0x04}},
                "username",
                error::bad_reply_size);
        }

        // wrong version
        {
            auto r = make_reply();
            r[0] = 0x05;
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                r,
                "username",
                error::bad_reply_version);
        }

        // request rejected
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_reply(
                    reply_code_v4::request_rejected_or_failed),
                "username",
                error::request_rejected_or_failed);
        }

        // unassigned reply code
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_reply(
                    static_cast<reply_code_v4>(0xEF)),
                "username",
                error::unassigned_reply_code);
        }

        // missing endpoint
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                std::array<unsigned char, 2>{{
                     0x04,
                     static_cast<unsigned char>(
                     reply_code_v4::request_granted)}},
                "username",
                error::bad_reply_size);
        }

        // write failure
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_reply(),
                "",
                error::general_failure,
                0,
                error::general_failure);
        }

        // read failure
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_reply(),
                "",
                error::general_failure,
                1,
                error::general_failure);
        }
    }

    template <
        class ConstBufferSequence,
        typename std::enable_if<
            asio::is_const_buffer_sequence<
                ConstBufferSequence
                >::value,
            int>::type = 0
        >
    static
    void
    checkAsyncEndpoint(
        ConstBufferSequence const& reply,
        string_view user,
        error_code exp_ec,
        std::size_t fail_at = 0,
        error_code fail_with = {},
        endpoint const& ep = {
            asio::ip::make_address_v4(
                asio::ip::address_v4::uint_type(0)),
                0})
    {
        io_context ioc;
        test::stream s(ioc, fail_at, fail_with);
        s.reset_read(reply);
        async_connect_v4(s, ep, user, [&](
            error_code ec, endpoint app_ep)
        {
            auto buf = make_request(ep, user);
            BOOST_TEST(s.equal_write_buffers(asio::buffer(buf)));
            BOOST_TEST_EQ(app_ep, ep);
            BOOST_TEST_EQ(ec, exp_ec);
        });
        ioc.run();
    };

    template <std::size_t N>
    static
    void
    checkAsyncEndpoint(
        std::array<unsigned char, N> const& reply,
        string_view user,
        error_code exp_ec,
        std::size_t fail_at = 0,
        error_code fail_with = {},
        endpoint const& ep = {
            asio::ip::make_address_v4(
                asio::ip::address_v4::uint_type(0)),
                0})
    {
        checkAsyncEndpoint(
            asio::buffer(reply),
            user,
            exp_ec,
            fail_at,
            fail_with,
            ep);
    }


    static
    void
    testAsyncEndpoint()
    {
        // no auth
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_reply(),
                "",
                error::request_granted);
        }

        // user
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_reply(),
                "username",
                error::request_granted);
        }

        // reply buf too small
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                std::array<unsigned char, 1>{{0x04}},
                "username",
                error::bad_reply_size);
        }

        // wrong version
        {
            auto r = make_reply();
            r[0] = 0x05;
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                r,
                "username",
                error::bad_reply_version);
        }

        // request rejected
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_reply(
                    reply_code_v4::request_rejected_or_failed),
                "username",
                error::request_rejected_or_failed);
        }

        // missing endpoint
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                std::array<unsigned char, 2>{{
                     0x04,
                     static_cast<unsigned char>(
                     error::request_granted)}},
                "username",
                error::bad_reply_size);
        }

        // write failure
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_reply(),
                "",
                error::general_failure,
                0,
                error::general_failure);
        }

        // read failure
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_reply(),
                "",
                error::general_failure,
                1,
                error::general_failure);
        }
    }

    void
    run()
    {
        testEndpoint();
        testAsyncEndpoint();
    }
};

TEST_SUITE(
    io_connect_v4_test,
    "boost.socks.connect_v4");

} // socks
} // boost
