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
#include <boost/socks/connect.hpp>
#include <boost/socks/detail/auth_method.hpp>
#include <boost/socks/detail/reply_code.hpp>
#include <array>
#include "stream.hpp"
#include "test_suite.hpp"

namespace boost {
namespace socks {

class io_connect_test
{
public:
    using io_context = asio::io_context;
    using endpoint = asio::ip::tcp::endpoint;
    using reply_code = detail::reply_code;
    using address_type = detail::address_type;
    using auth_method = detail::auth_method;

    static
    std::vector<unsigned char>
    make_greeting(auth_options const& opt)
    {
        std::vector<unsigned char> r(4);
        std::size_t n =
            detail::prepare_greeting(
                r.data(), r.size(), opt);
        r.resize(n);
        return r;
    }

    static
    std::array<unsigned char, 2>
    make_greet_reply(
        auth_method m = auth_method::no_authentication)
    {
        return {{0x05, static_cast<unsigned char>(m)}};
    }

    static
    std::vector<unsigned char>
    make_request(endpoint const& ep)
    {
        std::vector<unsigned char> r(22);
        std::size_t n =
            detail::prepare_request(
                r.data(), r.size(), ep);
        r.resize(n);
        return r;
    }

    static
    std::vector<unsigned char>
    make_request(detail::domain_endpoint_view const& ep)
    {
        std::vector<unsigned char> r(263);
        std::size_t n =
            detail::prepare_request(
                r.data(), r.size(), ep);
        r.resize(n);
        return r;
    }

    static
    std::array<unsigned char, 10>
    make_reply(reply_code r = reply_code::succeeded)
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
    std::array<unsigned char, 8>
    make_reply_incomplete(reply_code r = reply_code::succeeded)
    {
        return {{
             0x05, // VER
             static_cast<unsigned char>(r), // REP
             0x00, // RSV
             static_cast<unsigned char>(address_type::ip_v4), // ATYP
             0x00, // BND. ADDR
             0x00,
             0x00,
             0x00}};
    }

    static
    std::array<unsigned char, 22>
    make_reply_ipv6(
        reply_code r = reply_code::succeeded)
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

    static
    std::array<unsigned char, 20>
    make_reply_ipv6_incomplete(
        reply_code r = reply_code::succeeded)
    {
        return {{
             0x05, // VER
             static_cast<unsigned char>(r), // REP
             0x00, // RSV
             static_cast<unsigned char>(address_type::ip_v6), // ATYP
             // BND. ADDR
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
             // BND. PORT
             }};
    }

    static
    std::array<unsigned char, 22>
    make_reply_unknown(
        reply_code r = reply_code::succeeded)
    {
        return {{
             0x05, // VER
             static_cast<unsigned char>(r), // REP
             0x00, // RSV
             0xEF, // ATYP (unknown)
             // BND. ADDR
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             // BND. PORT
             0x00, 0x00}};
    }

    template <std::size_t N1, std::size_t N2>
    static
    void
    checkEndpoint(
        std::array<unsigned char, N1> const& greet_reply,
        std::array<unsigned char, N2> const& reply,
        auth_options const& auth,
        error_code exp_ec,
        std::size_t fail_at = 0,
        error_code fail_with = {},
        endpoint const& ep = {
            asio::ip::make_address_v4(
                asio::ip::address_v4::uint_type(0)),
                0})
    {
        io_context ioc;
        // Mock proxy response
        test::stream s(ioc, fail_at, fail_with);
        s.reset_read(greet_reply.data(), greet_reply.size());
        s.append_read(reply.data(), reply.size());
        // Connect to proxy server
        error_code ec;
        endpoint app_ep = connect(s, ep, auth, ec);
        // Compare to expected write values
        if (!ec.failed())
        {
            auto buf1 = make_greeting(auth);
            auto buf2 = make_request(ep);
            BOOST_TEST(s.equal_write_buffers(
                std::array<asio::const_buffer, 2>{
                    asio::buffer(buf1),
                    asio::buffer(buf2) }));
        }
        BOOST_TEST_EQ(app_ep, ep);
        BOOST_TEST_EQ(ec, exp_ec);
    };

    static
    void
    testEndpoint()
    {
        // no auth
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                make_reply(),
                auth_options::none{},
                error::succeeded);
        }

        // user
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(
                    auth_method::userpass),
                make_reply(),
                auth_options::userpass{"user", "pass"},
                error::succeeded);
        }

        // successful ipv6
        {
            asio::ip::address_v6::bytes_type bytes;
            bytes.fill(0x00);
            endpoint ep(asio::ip::make_address_v6(bytes), 0);
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                make_reply_ipv6(),
                auth_options::none{},
                error::succeeded,
                0,
                {},
                ep
            );
        }

        // wrong ATYP - ipv6
        {
            auto reply = make_reply_ipv6();
            reply[3] = static_cast<unsigned char>(
                address_type::ip_v4);
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                reply,
                auth_options::none{},
                error::bad_reply_size
            );
        }

        // wrong ATYP - ipv4
        {
            auto reply = make_reply();
            reply[3] = static_cast<unsigned char>(
                address_type::ip_v6);
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                reply,
                auth_options::none{},
                error::bad_reply_size
            );
        }

        // incomplete greet reply version
        {
            auto g = make_greet_reply();
            g[0] = 0x04;
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g,
                make_reply(),
                auth_options::none{},
                error::bad_reply_version,
                0,
                {}
            );
        }

        // incomplete greet reply method
        {
            auto g = make_greet_reply();
            g[1] = 0xEF;
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                g,
                make_reply(),
                auth_options::none{},
                error::bad_server_choice,
                0,
                {}
            );
        }

        // incomplete greet reply
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                std::array<unsigned char, 1>{{0x05}},
                std::array<unsigned char, 0>{{}},
                auth_options::none{},
                error::bad_reply_size,
                0,
                {}
            );
        }

        // incomplete ipv4 reply
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                make_reply_incomplete(),
                auth_options::none{},
                error::bad_reply_size,
                0,
                {}
            );
        }

        // incomplete ipv6 reply
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                make_reply_ipv6_incomplete(),
                auth_options::none{},
                error::bad_reply_size,
                0,
                {}
            );
        }

        // unknown address type in reply
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                make_reply_unknown(),
                auth_options::none{},
                error::bad_address_type,
                0,
                {}
            );
        }

        // invalid/unassigned reply code
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                make_reply(static_cast<reply_code>(0xEF)),
                auth_options::none{},
                error::unassigned_reply_code,
                0,
                {}
            );
        }

        // successful domain name
        {
            io_context ioc;
            // Mock proxy response
            test::stream s(ioc);
            auth_options::none auth;
            auto greet_reply = make_greet_reply(
                auth_method::no_authentication);
            s.reset_read(greet_reply.data(), greet_reply.size());
            auto reply = make_reply(reply_code::succeeded);
            s.append_read(reply.data(), reply.size());
            // Connect to proxy server
            error_code ec;
            endpoint app_ep =
                connect(s, "www.example.com", 80, auth, ec);
            // Compare to expected write values
            auto buf1 = make_greeting(auth);
            detail::domain_endpoint_view ep;
            ep.domain  = "www.example.com";
            ep.port = 80;
            auto buf2 = make_request(ep);
            BOOST_TEST(s.equal_write_buffers(
                std::array<asio::const_buffer, 2>{
                    asio::buffer(buf1), asio::buffer(buf2)}));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, error::succeeded);
        }

        // authentication failure - domain name
        {
            io_context ioc;
            // Mock proxy response
            test::stream s(ioc);
            auth_options::none auth;
            auto greet_reply = make_greet_reply(
                auth_method::no_authentication);
            greet_reply[0] = 0x04;
            s.reset_read(greet_reply.data(), greet_reply.size());
            auto reply = make_reply(reply_code::succeeded);
            s.append_read(reply.data(), reply.size());
            // Connect to proxy server
            error_code ec;
            endpoint app_ep =
                connect(s, "www.example.com", 80, auth, ec);
            // Compare to expected write values
            auto buf1 = make_greeting(auth);
            detail::domain_endpoint_view ep;
            ep.domain  = "www.example.com";
            ep.port = 80;
            BOOST_TEST(s.equal_write_buffers(
                asio::buffer(buf1)));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, error::bad_reply_version);
        }

        // write connect failure - domain name
        {
            io_context ioc;
            // Mock proxy response
            test::stream s(ioc, 2, error::general_failure);
            auth_options::none auth;
            auto greet_reply = make_greet_reply(
                auth_method::no_authentication);
            s.reset_read(greet_reply.data(), greet_reply.size());
            auto reply = make_reply(reply_code::succeeded);
            s.append_read(reply.data(), reply.size());
            // Connect to proxy server
            error_code ec;
            endpoint app_ep =
                connect(s, "www.example.com", 80, auth, ec);
            // Compare to expected write values
            auto buf1 = make_greeting(auth);
            detail::domain_endpoint_view ep;
            ep.domain  = "www.example.com";
            ep.port = 80;
            auto buf2 = make_request(ep);
            BOOST_TEST(s.equal_write_buffers(
                std::array<asio::const_buffer, 2>{
                    asio::buffer(buf1), asio::buffer(buf2)}));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, error::general_failure);
        }

        // read connect failure - domain name
        {
            io_context ioc;
            // Mock proxy response
            test::stream s(ioc, 3, error::general_failure);
            auth_options::none auth;
            auto greet_reply = make_greet_reply(
                auth_method::no_authentication);
            s.reset_read(greet_reply.data(), greet_reply.size());
            auto reply = make_reply(reply_code::succeeded);
            s.append_read(reply.data(), reply.size());
            // Connect to proxy server
            error_code ec;
            endpoint app_ep =
                connect(s, "www.example.com", 80, auth, ec);
            // Compare to expected write values
            auto buf1 = make_greeting(auth);
            detail::domain_endpoint_view ep;
            ep.domain  = "www.example.com";
            ep.port = 80;
            auto buf2 = make_request(ep);
            BOOST_TEST(s.equal_write_buffers(
                std::array<asio::const_buffer, 2>{
                    asio::buffer(buf1), asio::buffer(buf2)}));
            BOOST_TEST_EQ(app_ep.address().to_v4(),
                          asio::ip::make_address_v4("0.0.0.0"));
            BOOST_TEST_EQ(ec, error::general_failure);
        }

        // reply buf too small
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                std::array<unsigned char, 1>{{0x05}},
                auth_options::none{},
                error::bad_reply_size);
        }

        // wrong version
        {
            auto r = make_reply(
                reply_code::succeeded);
            r[0] = 0x04;
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                r,
                auth_options::none{},
                error::bad_reply_version);
        }

        // request rejected
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                make_reply(
                    reply_code::general_failure),
                auth_options::none{},
                error::general_failure);
        }

        // missing endpoint
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                std::array<unsigned char, 2>{{
                     0x05,
                     static_cast<unsigned char>(
                     reply_code::succeeded)}},
                auth_options::none{},
                error::bad_reply_size);
        }

        // write failure
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                std::array<unsigned char, 0>{{}},
                std::array<unsigned char, 0>{{}},
                auth_options::none{},
                error::general_failure,
                0,
                error::general_failure);
        }

        // read failure
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                std::array<unsigned char, 0>{{}},
                std::array<unsigned char, 0>{{}},
                auth_options::none{},
                error::general_failure,
                1,
                error::general_failure);
        }

        // 2nd write failure
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                std::array<unsigned char, 0>{{}},
                auth_options::none{},
                error::general_failure,
                2,
                error::general_failure);
        }

        // 2nd read failure
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                make_greet_reply(),
                make_reply(),
                auth_options::none{},
                error::general_failure,
                3,
                error::general_failure);
        }
    }

    template <std::size_t N1, std::size_t N2>
    static
    void
    checkAsyncEndpoint(
        std::array<unsigned char, N1> greet_reply,
        std::array<unsigned char, N2> reply,
        auth_options const& auth,
        error_code exp_ec,
        std::size_t fail_at = 0,
        error_code fail_with = {},
        endpoint const& ep = {
             asio::ip::make_address_v4(
                asio::ip::address_v4::uint_type(0)),
                0})
    {
        io_context ioc;
        // Mock proxy response
        test::stream s(ioc, fail_at, fail_with);
        s.reset_read(greet_reply.data(), greet_reply.size());
        s.append_read(reply.data(), reply.size());
        // Connect to proxy server
        async_connect(s, ep, auth,
            [&](error_code ec, endpoint app_ep)
        {
            // Compare to expected write values
            auto buf1 = make_greeting(auth);
            std::vector<unsigned char> buf2;
            if (!fail_with.failed() || fail_at >= 2)
                buf2 = make_request(ep);
            BOOST_TEST(s.equal_write_buffers(
                std::array<asio::const_buffer, 2>{
                    asio::buffer(buf1),
                    asio::buffer(buf2)}));
            BOOST_TEST_EQ(app_ep, ep);
            BOOST_TEST_EQ(ec, exp_ec);
        });
        ioc.run();
    }

    static
    void
    testAsyncEndpoint()
    {
        // no auth
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_greet_reply(),
                make_reply(),
                auth_options::none{},
                error::succeeded);
        }

        // user
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_greet_reply(
                    auth_method::userpass),
                make_reply(),
                auth_options::userpass{"user", "pass"},
                error::succeeded);
        }

        // reply buf too small
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_greet_reply(),
                std::array<unsigned char, 1>{{0x05}},
                auth_options::none{},
                error::bad_reply_size);
        }

        // wrong version
        {
            auto r = make_reply(
                reply_code::succeeded);
            r[0] = 0x04;
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_greet_reply(),
                r,
                auth_options::none{},
                error::bad_reply_version);
        }

        // request rejected
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_greet_reply(),
                make_reply(
                    reply_code::connection_refused),
                auth_options::none{},
                error::connection_refused);
        }

        // missing reply endpoint
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_greet_reply(),
                std::array<unsigned char, 3>{{
                    0x05, // VER
                    static_cast<unsigned char>(
                        reply_code::succeeded), // REP
                    0x00 // RSV
                }},
                auth_options::none{},
                error::bad_reply_size);
        }

        // write failure
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                std::array<unsigned char, 0>{{}},
                std::array<unsigned char, 0>{{}},
                auth_options::none{},
                error::general_failure,
                0,
                error::general_failure);
        }

        // read failure
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_greet_reply(),
                std::array<unsigned char, 0>{{}},
                auth_options::none{},
                error::general_failure,
                1,
                error::general_failure);
        }

        // 2nd write failure
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_greet_reply(),
                std::array<unsigned char, 0>{{}},
                auth_options::none{},
                error::general_failure,
                2,
                error::general_failure);
        }

        // 2nd read failure
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                make_greet_reply(),
                make_reply(),
                auth_options::none{},
                error::general_failure,
                3,
                error::general_failure);
        }

        // successful domain name
        {
            io_context ioc;
            test::stream s(ioc);
            auth_options::none auth;
            auto greet_reply = make_greet_reply(
                auth_method::no_authentication);
            s.reset_read(greet_reply.data(), greet_reply.size());
            auto reply = make_reply(reply_code::succeeded);
            s.append_read(reply.data(), reply.size());
            async_connect(
                s, "www.example.com", 80, auth,
                [](error_code ec, endpoint app_ep)
                {
                    BOOST_TEST_EQ(app_ep.address().to_v4(),
                                  asio::ip::make_address_v4("0.0.0.0"));
                    BOOST_TEST_EQ(ec, error::succeeded);
                });
            ioc.run();
        }

        // fail to read - domain name
        {
            io_context ioc;
            test::stream s(ioc, 1, error::general_failure);
            s.reset_read(nullptr, 0);
            endpoint ep(asio::ip::make_address_v4(127), 0);
            async_connect(
                s, "www.example.com", 80, auth_options::none{},
                [](error_code ec, endpoint app_ep)
                {
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, error::general_failure);
                });
            ioc.run();
        }

        // fail to write - domain name
        {
            io_context ioc;
            test::stream s(ioc, 1, error::general_failure);
            auto r = make_reply(
                reply_code::succeeded);
            s.reset_read(r.data(), r.size());
            endpoint ep(asio::ip::make_address_v4(127), 0);
            async_connect(
                s, "www.example.com", 80, auth_options::none{},
                [](error_code ec, endpoint app_ep)
                {
                BOOST_TEST_EQ(app_ep.address().to_v4(),
                              asio::ip::make_address_v4("0.0.0.0"));
                BOOST_TEST_EQ(ec, error::general_failure);
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
    "boost.socks.connect");

} // socks
} // boost
