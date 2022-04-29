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
#include <boost/socks_proto/io/connect.hpp>
#include <array>
#include "stream.hpp"
#include "test_suite.hpp"

namespace boost {
namespace socks_proto {

class io_connect_test
{
public:
    using io_context = asio::io_context;
    using endpoint = asio::ip::tcp::endpoint;

    static
    std::array<unsigned char, 2>
    make_greet_reply(auth_method m)
    {
        return {{0x05, static_cast<unsigned char>(m)}};
    }

    static
    std::array<unsigned char, 10>
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
             0x00,
             0x00, // BND. PORT
             0x00}};
    }

    static
    std::array<unsigned char, 22>
    make_v5_reply_ipv6(reply_code r)
    {
        return {{
             0x05, // VER
             static_cast<unsigned char>(r), // REP
             0x00, // RSV
             static_cast<unsigned char>(address_type::ip_v6), // ATYP
             // BND. ADDR
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             // BND. PORT
             0x00, 0x00}};
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
        endpoint app_ep = io::connect(s, ep, auth, ec);
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
            auto g = make_greet_reply(
                auth_method::no_authentication);
            auto r = make_v5_reply(
                reply_code::succeeded);
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                reply_code::succeeded);
        }

        // user
        {
            io::auth::userpass a{"user", "pass"};
            auto g = make_greet_reply(
                auth_method::userpass);
            auto r = make_v5_reply(
                reply_code::succeeded);
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                a,
                reply_code::succeeded);
        }

        // successful ipv6
        {
            auto g = make_greet_reply(
                auth_method::no_authentication);
            auto r = make_v5_reply_ipv6(
                reply_code::succeeded);
            io_context ioc;
            // Mock proxy response
            test::stream s(ioc);
            s.reset_read(g.data(), g.size());
            s.append_read(r.data(), r.size());
            // Connect to proxy server
            asio::ip::address_v6::bytes_type bytes;
            bytes.fill(0x00);
            endpoint ep(asio::ip::make_address_v6(bytes), 0);
            error_code ec;
            auto auth = io::auth::no_auth{};
            endpoint app_ep = io::connect(s, ep, auth, ec);
            // Compare to expected write values
            auto buf1 = io::detail::prepare_greeting(auth);
            auto buf2 = io::detail::prepare_request_v5(ep);
            BOOST_TEST(s.equal_write_buffers(
                std::array<asio::const_buffer, 2>{
                    asio::buffer(buf1), asio::buffer(buf2)}));
            BOOST_TEST_EQ(app_ep.address().to_v6(),
                          asio::ip::make_address_v6(bytes));
            BOOST_TEST_EQ(ec, reply_code::succeeded);
        }

        // reply buf too small
        {
            auto g = make_greet_reply(
                auth_method::no_authentication);
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
            auto g = make_greet_reply(
                auth_method::no_authentication);
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
            auto g = make_greet_reply(
                auth_method::no_authentication);
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
            auto g = make_greet_reply(
                auth_method::no_authentication);
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
            auto g = make_greet_reply(
                auth_method::no_authentication);
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
                reply_code::succeeded);
        }

        // write failure
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_write(0);
            s.reset_write_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            io::auth::no_auth auth{};
            endpoint app_ep = io::connect(s, ep, auth, ec);
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, asio::error::no_permission);
        }

        // 2nd write failure
        {
            io_context ioc;
            test::stream s(ioc);
            auto g = make_greet_reply(
                auth_method::no_authentication);
            s.reset_read(g.data(), g.size());
            s.reset_write(3);
            s.reset_write_ec2(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            io::auth::no_auth auth{};
            endpoint app_ep = io::connect(s, ep, auth, ec);
            auto buf1 = io::detail::prepare_greeting(auth);
            BOOST_TEST(s.equal_write_buffers(asio::buffer(buf1)));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, asio::error::no_permission);
        }

        // read failure
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            io::auth::no_auth auth{};
            endpoint app_ep = io::connect(s, ep, io::auth::no_auth{}, ec);
            auto buf1 = io::detail::prepare_greeting(auth);
            BOOST_TEST(s.equal_write_buffers(asio::buffer(buf1)));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, asio::error::no_permission);
        }

        // 2nd read failure
        {
            io_context ioc;
            test::stream s(ioc);
            auto g = make_greet_reply(
                auth_method::no_authentication);
            s.reset_read(g.data(), g.size());
            s.reset_read_ec2(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            io::auth::no_auth auth{};
            endpoint app_ep = io::connect(s, ep, io::auth::no_auth{}, ec);
            auto buf1 = io::detail::prepare_greeting(auth);
            auto buf2 = io::detail::prepare_request_v5(ep);
            BOOST_TEST(s.equal_write_buffers(
                std::array<asio::const_buffer, 2>{
                    asio::buffer(buf1), asio::buffer(buf2)}));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, asio::error::no_permission);
        }

        // success ipv6
        {
            io_context ioc;
            // Mock proxy response
            test::stream s(ioc);
            auto greet_reply = make_greet_reply(
                auth_method::no_authentication);
            auto reply = make_v5_reply_ipv6(
                reply_code::succeeded);
            s.reset_read(greet_reply.data(), greet_reply.size());
            s.append_read(reply.data(), reply.size());
            // Connect to proxy server
            asio::ip::address_v6::bytes_type ip_bytes;
            ip_bytes.fill(0);
            endpoint ep(asio::ip::make_address_v6(ip_bytes), 0);
            error_code ec;
            io::auth::no_auth auth{};
            endpoint app_ep = io::connect(s, ep, auth, ec);
            // Compare to expected write values
            auto buf1 = io::detail::prepare_greeting(auth);
            auto buf2 = io::detail::prepare_request_v5(ep);
            BOOST_TEST(s.equal_write_buffers(
                std::array<asio::const_buffer, 2>{
                    asio::buffer(buf1), asio::buffer(buf2)}));
            BOOST_TEST(app_ep.address().is_v6());
            BOOST_TEST_EQ(app_ep.address().to_v6(),
                          asio::ip::make_address_v6(ip_bytes));
            BOOST_TEST_EQ(ec, reply_code::succeeded);
        }

        // successful hostname
        {
            io_context ioc;
            // Mock proxy response
            test::stream s(ioc);
            io::auth::no_auth auth;
            auto greet_reply = make_greet_reply(
                auth_method::no_authentication);
            s.reset_read(greet_reply.data(), greet_reply.size());
            auto reply = make_v5_reply(reply_code::succeeded);
            s.append_read(reply.data(), reply.size());
            // Connect to proxy server
            error_code ec;
            endpoint app_ep =
                io::connect(s, "www.example.com", 80, auth, ec);
            // Compare to expected write values
            auto buf1 = io::detail::prepare_greeting(auth);
            std::pair<string_view, std::uint16_t> ep("www.example.com", 80);
            auto buf2 = io::detail::prepare_request_v5(ep);
            BOOST_TEST(s.equal_write_buffers(
                std::array<asio::const_buffer, 2>{
                    asio::buffer(buf1), asio::buffer(buf2)}));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, reply_code::succeeded);
        }

        // read failure
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            endpoint app_ep = io::connect(
                s, "www.example.com", 80, io::auth::no_auth{}, ec);
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, asio::error::no_permission);
        }

        // write failure
        {
            io_context ioc;
            test::stream s(ioc);
            auto r = make_v5_reply(
                reply_code::succeeded);
            s.reset_read(r.data(), r.size());
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            error_code ec;
            endpoint app_ep = io::connect(
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
        io::async_connect(s, ep, auth,
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
            auto g = make_greet_reply(
                auth_method::no_authentication);
            auto r = make_v5_reply(
                reply_code::succeeded);
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::no_auth{},
                reply_code::succeeded);
        }

        // user
        {
            auto g = make_greet_reply(
                auth_method::userpass);
            auto r = make_v5_reply(
                reply_code::succeeded);
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                g.data(),
                g.size(),
                r.data(),
                r.size(),
                io::auth::userpass{"user", "pass"},
                reply_code::succeeded);
        }

        // reply buf too small
        {
            auto g = make_greet_reply(
                auth_method::no_authentication);
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
            auto g = make_greet_reply(
                auth_method::no_authentication);
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
            auto g = make_greet_reply(
                auth_method::no_authentication);
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

        // missing reply endpoint
        {
            auto g = make_greet_reply(
                auth_method::no_authentication);
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
                reply_code::succeeded);
        }

        // write failure
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_write_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            io::async_connect(s, ep, io::auth::no_auth{},
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

        // 2nd write failure
        {
            io_context ioc;
            test::stream s(ioc);
            auto g = make_greet_reply(
                auth_method::no_authentication);
            s.reset_read(g.data(), g.size());
            s.reset_write_ec2(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            io::async_connect(s, ep, io::auth::no_auth{},
                                 [&](error_code ec, endpoint app_ep)
                                 {
                auto buf1 = io::detail::prepare_greeting(io::auth::no_auth{});
                auto buf2 = io::detail::prepare_request_v5(ep);
                BOOST_TEST(s.equal_write_buffers(
                    std::array<asio::const_buffer, 2>{
                        asio::buffer(buf1), asio::buffer(buf2)}));
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, asio::error::no_permission);
            });
            ioc.run();
        }

        // read failure
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            io::async_connect(s, ep, io::auth::no_auth{},
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

        // 2nd read failure
        {
            io_context ioc;
            test::stream s(ioc);
            auto g = make_greet_reply(
                auth_method::no_authentication);
            auto r = make_v5_reply(
                reply_code::succeeded);
            s.reset_read(g.data(), g.size());
            s.append_read(r.data(), r.size());
            s.reset_read_ec2(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            io::async_connect(s, ep, io::auth::no_auth{},
                                 [&](error_code ec, endpoint app_ep)
                                 {
                auto buf1 = io::detail::prepare_greeting(io::auth::no_auth{});
                auto buf2 = io::detail::prepare_request_v5(ep);
                BOOST_TEST(s.equal_write_buffers(
                    std::array<asio::const_buffer, 2>{
                        asio::buffer(buf1), asio::buffer(buf2)}));
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, asio::error::no_permission);
            });
            ioc.run();
        }

        // fail to read
        {
            io_context ioc;
            test::stream s(ioc);
            s.reset_read(nullptr, 0);
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            io::async_connect(
                s, "www.example.com", 80, io::auth::no_auth{},
                [](error_code ec, endpoint app_ep)
                {
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, make_error_code(asio::error::no_permission));
                });
            ioc.run();
        }

        // fail to write
        {
            io_context ioc;
            test::stream s(ioc);
            auto r = make_v5_reply(
                reply_code::succeeded);
            s.reset_read(r.data(), r.size());
            s.reset_read_ec(asio::error::no_permission);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            io::async_connect(
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
        testAsyncEndpoint();
    }
};

TEST_SUITE(
    io_connect_test,
    "boost.socks_proto.io.connect");

} // socks_proto
} // boost
