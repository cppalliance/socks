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
#include <boost/socks_proto/io/connect_v5.hpp>
#include "test_suite.hpp"
#include "stream.hpp"
#include <array>

namespace boost {
namespace socks_proto {

class io_connect_v5_test
{
public:
    using io_context = asio::io_context;
    using endpoint = asio::ip::tcp::endpoint;

    static
    std::array<unsigned char, 9>
    make_v5_reply(reply_code r)
    {
        return {{
             0x05, // VER
             static_cast<unsigned char>(r), // REP
             0x00, // RSV
             static_cast<unsigned char>(address_type::ip_v4), // ATYP
             0x00, // BND. ADDR
             0x00,
             0x00,
             0x00, // BND. PORT
             0x00}};
    }

    template <class AuthOpt>
    static
    void
    checkEndpoint(
        unsigned char const* greet_reply,
        std::size_t greet_reply_n,
        unsigned char const* reply,
        std::size_t reply_n,
        AuthOpt auth,
        error_code exp_ec)
    {
        io_context ioc;
        // Mock proxy response
        test::stream s(ioc);
        s.reset_read(greet_reply, greet_reply_n);
        s.append_read(reply, reply_n);
        // Connect to proxy server
        endpoint ep(asio::ip::make_address_v4(127), 0);
        error_code ec;
        endpoint app_ep = io::connect_v5(s, ep, auth, ec);
        // Compare to expected write values
        auto buf1 = io::detail::prepare_greeting(auth);
        auto buf2 = io::detail::prepare_request_v5(ep);
        BOOST_TEST(s.equal_write_buffers(
            std::array<asio::const_buffer, 2>{
                asio::buffer(buf1), asio::buffer(buf2)}));
        BOOST_TEST_EQ(app_ep.address().to_v4(),
                      asio::ip::make_address_v4("0.0.0.0"));
        BOOST_TEST_EQ(ec, exp_ec);
    };

    static
    void
    testEndpoint()
    {
        // no auth
        {
            auto g = io::detail::prepare_greeting(
                    io::auth::no_auth{});
            auto r = make_v5_reply(
                reply_code::succeeded);
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                {});
        }

        // user
        {
            io::auth::userpass a{"user", "pass"};
            auto g = io::detail::prepare_greeting(a);
            auto r = make_v5_reply(
                reply_code::succeeded);
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                a,
                {});
        }

        // reply buf too small
        {
            auto g = io::detail::prepare_greeting(
                io::auth::no_auth{});
            std::array<unsigned char, 1> r = {{0x05}};
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                asio::error::access_denied);
        }

        // wrong version
        {
            auto g = io::detail::prepare_greeting(
                io::auth::no_auth{});
            auto r = make_v5_reply(
                reply_code::succeeded);
            r[0] = 0x04;
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                asio::error::no_protocol_option);
        }

        // wrong version
        {
            auto g = io::detail::prepare_greeting(
                    io::auth::no_auth{});
            auto r = make_v5_reply(
                reply_code::succeeded);
            r[0] = 0x04;
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                asio::error::no_protocol_option);
        }

        // request rejected
        {
            auto g = io::detail::prepare_greeting(
                    io::auth::no_auth{});
            auto r = make_v5_reply(
                reply_code::general_failure);
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                reply_code::general_failure);
        }

        // missing endpoint
        {
            auto g = io::detail::prepare_greeting(
                io::auth::no_auth{});
            std::array<unsigned char, 2> r = {{
                 0x05,
                 static_cast<unsigned char>(
                 reply_code::succeeded)}};
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                {});
        }

        // failure when writing
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_write_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            io::auth::no_auth auth{};
            endpoint app_ep = io::connect_v5(s, ep, auth, ec);
            auto buf1 = io::detail::prepare_greeting(auth);
            BOOST_TEST(s.equal_write_buffers(asio::buffer(buf1)));
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
            io::auth::no_auth auth{};
            endpoint app_ep = io::connect_v5(s, ep, io::auth::no_auth{}, ec);
            auto buf1 = io::detail::prepare_greeting(auth);
            BOOST_TEST(s.equal_write_buffers(asio::buffer(buf1)));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, asio::error::no_permission);
        }
    }

    void
    testHostname()
    {
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            endpoint app_ep = io::connect_v5(
                s, "www.example.com", 80, io::auth::no_auth{}, ec);
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, asio::error::no_permission);
        }

        {
            io_context ioc;
            test::stream s(ioc);
            auto r = make_v5_reply(
                reply_code::succeeded);
            s.reset_read(r.data(), r.size());
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            endpoint app_ep = io::connect_v5(
                s, "www.example.com", 80, io::auth::no_auth{}, ec);
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, asio::error::no_permission);
        }
    }

    template <class Auth>
    static
    void
    checkAsyncEndpoint(
        unsigned char const* greet_reply,
        std::size_t greet_reply_n,
        unsigned char const* reply,
        std::size_t reply_n,
        Auth auth,
        error_code exp_ec)
    {
        io_context ioc;
        // Mock proxy response
        test::stream s(ioc);
        s.reset_read(greet_reply, greet_reply_n);
        s.append_read(reply, reply_n);
        // Connect to proxy server
        endpoint ep(asio::ip::make_address_v4(127), 0);
        io::async_connect_v5(s, ep, auth,
            [&](error_code ec, endpoint app_ep)
        {
            // Compare to expected write values
            auto buf1 = io::detail::prepare_greeting(auth);
            auto buf2 = io::detail::prepare_request_v5(ep);
            BOOST_TEST(s.equal_write_buffers(
                std::array<asio::const_buffer, 2>{
                    asio::buffer(buf1), asio::buffer(buf2)}));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, exp_ec);
        });
        ioc.run();
    }

    void
    testAsyncEndpoint()
    {
        // no auth
        {
            auto g = io::detail::prepare_greeting(
                io::auth::no_auth{});
            auto r = make_v5_reply(
                reply_code::succeeded);
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                {});
        }

        // user
        {
            auto g = io::detail::prepare_greeting(
                io::auth::no_auth{});
            auto r = make_v5_reply(
                reply_code::succeeded);
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::userpass{"user", "pass"},
                {});
        }

        // reply buf too small
        {
            auto g = io::detail::prepare_greeting(
                io::auth::no_auth{});
            std::array<unsigned char, 1> r = {{0x05}};
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                asio::error::access_denied);
        }

        // wrong version
        {
            auto g = io::detail::prepare_greeting(
                io::auth::no_auth{});
            auto r = make_v5_reply(
                reply_code::succeeded);
            r[0] = 0x04;
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                asio::error::no_protocol_option);
        }

        // wrong version
        {
            auto g = io::detail::prepare_greeting(
                io::auth::no_auth{});
            auto r = make_v5_reply(
                reply_code::succeeded);
            r[0] = 0x04;
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                asio::error::no_protocol_option);
        }

        // request rejected
        {
            auto g = io::detail::prepare_greeting(
                io::auth::no_auth{});
            auto r = make_v5_reply(
                reply_code::connection_refused);
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                reply_code::connection_refused);
        }

        // missing endpoint
        {
            auto g = io::detail::prepare_greeting(
                io::auth::no_auth{});
            std::array<unsigned char, 3> r = {{
                 0x05, // VER
                 static_cast<unsigned char>(
                 reply_code::succeeded), // REP
                 0x00 // RSV
            }};
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                {});
        }

        // failure when writing
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_write_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            io::async_connect_v5(s, ep, io::auth::no_auth{},
                                 [&](error_code ec, endpoint app_ep)
                                 {
                auto buf = io::detail::prepare_greeting(io::auth::no_auth{});
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
            io::async_connect_v5(s, ep, io::auth::no_auth{},
                                 [&](error_code ec, endpoint app_ep)
                                 {
                auto buf = io::detail::prepare_greeting(io::auth::no_auth{});
                BOOST_TEST(s.equal_write_buffers(asio::buffer(buf)));
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, asio::error::no_permission);
            });
            ioc.run();
        }
    }

    void
    testAsyncHostname()
    {
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            io::async_connect_v5(
                s, "www.example.com", 80, io::auth::no_auth{},
                [](error_code ec, endpoint app_ep)
                {
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, make_error_code(asio::error::no_permission));
                });
            ioc.run();
        }

        {
            io_context ioc;
            test::stream s(ioc);
            auto r = make_v5_reply(
                reply_code::succeeded);
            s.reset_read(r.data(), r.size());
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            io::async_connect_v5(
                s, "www.example.com", 80, io::auth::no_auth{},
                [](error_code ec, endpoint app_ep)
                {
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, make_error_code(asio::error::no_permission));
                });
            ioc.run();
        }
    }

    void
    run()
    {
        testEndpoint();
        testHostname();
        testAsyncEndpoint();
        testAsyncHostname();
    }
};

TEST_SUITE(
    io_connect_v5_test,
    "boost.socks_proto.io.connect_v5");

} // socks_proto
} // boost
