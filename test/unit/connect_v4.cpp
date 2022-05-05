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
    make_v4_reply(detail::reply_code_v4 r)
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

    static
    void
    testEndpoint()
    {
        auto check = [](
            unsigned char const* reply,
            std::size_t reply_n,
            string_view user,
            error_code exp_ec)
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(reply, reply_n);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            endpoint app_ep = connect_v4(s, ep, user, ec);
            auto buf = make_request(ep, user);
            BOOST_TEST(s.equal_write_buffers(asio::buffer(buf)));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, exp_ec);
        };

        // no auth
        {
            auto r = make_v4_reply(
                reply_code_v4::request_granted);
            BOOST_TEST_CHECKPOINT();
            check(r.data(), r.size(), "", {});
        }

        // user
        {
            auto r = make_v4_reply(
                reply_code_v4::request_granted);
            BOOST_TEST_CHECKPOINT();
            check(r.data(), r.size(), "username", {});
        }

        // reply buf too small
        {
            std::array<unsigned char, 1> r = {{0x04}};
            BOOST_TEST_CHECKPOINT();
            check(
                r.data(),
                r.size(),
                "username",
                error::bad_reply_size);
        }

        // wrong version
        {
            auto r = make_v4_reply(
                reply_code_v4::request_granted);
            r[0] = 0x05;
            BOOST_TEST_CHECKPOINT();
            check(
                r.data(),
                r.size(),
                "username",
                error::bad_reply_version);
        }

        // request rejected
        {
            auto r = make_v4_reply(
                reply_code_v4::request_rejected_or_failed);
            BOOST_TEST_CHECKPOINT();
            check(
                r.data(),
                r.size(),
                "username",
                error::request_rejected_or_failed);
        }

        // missing endpoint
        {
            std::array<unsigned char, 2> r = {{
                0x04,
                static_cast<unsigned char>(
                    reply_code_v4::request_granted)
            }};
            BOOST_TEST_CHECKPOINT();
            check(
                r.data(),
                r.size(),
                "username",
                error::bad_reply_size);
        }

        // failure when writing
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_write_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            endpoint app_ep = connect_v4(s, ep, "", ec);
            auto buf = make_request(ep, "");
            BOOST_TEST(s.equal_write_buffers(asio::buffer(buf)));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, asio::error::no_permission);
        }

        // failure when reading
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            endpoint app_ep = connect_v4(s, ep, "", ec);
            auto buf = make_request(ep, "");
            BOOST_TEST(s.equal_write_buffers(asio::buffer(buf)));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, asio::error::no_permission);
        }
    }

    void
    testAsyncEndpoint()
    {
        auto check = [](
            unsigned char const* reply,
            std::size_t reply_n,
            string_view user,
            error_code exp_ec)
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(reply, reply_n);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            async_connect_v4(s, ep, user, [&](
                error_code ec, endpoint app_ep)
            {
                auto buf = make_request(ep, user);
                BOOST_TEST(s.equal_write_buffers(asio::buffer(buf)));
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, exp_ec);
            });
            ioc.run();
        };

        // no auth
        {
            auto r = make_v4_reply(
                reply_code_v4::request_granted);
            BOOST_TEST_CHECKPOINT();
            check(r.data(), r.size(), "", {});
        }

        // user
        {
            auto r = make_v4_reply(
                reply_code_v4::request_granted);
            BOOST_TEST_CHECKPOINT();
            check(r.data(), r.size(), "username", {});
        }

        // reply buf too small
        {
            std::array<unsigned char, 1> r = {{0x04}};
            BOOST_TEST_CHECKPOINT();
            check(
                r.data(),
                r.size(),
                "username",
                error::bad_reply_size);
        }

        // wrong version
        {
            auto r = make_v4_reply(
                reply_code_v4::request_granted);
            r[0] = 0x05;
            BOOST_TEST_CHECKPOINT();
            check(
                r.data(),
                r.size(),
                "username",
                error::bad_reply_version);
        }

        // request rejected
        {
            auto r = make_v4_reply(
                reply_code_v4::request_rejected_or_failed);
            BOOST_TEST_CHECKPOINT();
            check(
                r.data(),
                r.size(),
                "username",
                error::request_rejected_or_failed);
        }

        // missing endpoint
        {
            std::array<unsigned char, 2> r = {{
                0x04,
                static_cast<unsigned char>(
                    error::request_granted)
            }};
            BOOST_TEST_CHECKPOINT();
            check(
                r.data(),
                r.size(),
                "username",
                error::bad_reply_size);
        }

        // failure when writing
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_write_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            async_connect_v4(s, ep, "",
                [&](error_code ec, endpoint app_ep)
            {
                auto buf = make_request(ep, "");
                BOOST_TEST(s.equal_write_buffers(asio::buffer(buf)));
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, asio::error::no_permission);
            });
            ioc.run();
        }

        // failure when reading
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            async_connect_v4(s, ep, "",
                [&](error_code ec, endpoint app_ep)
            {
                auto buf = make_request(ep, "");
                BOOST_TEST(s.equal_write_buffers(asio::buffer(buf)));
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, asio::error::no_permission);
            });
            ioc.run();
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
    "boost.socks.io.connect_v4");

} // socks
} // boost
