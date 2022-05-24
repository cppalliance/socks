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
    make_greeting(
        auth_options const& opt = auth_options::none{})
    {
        std::vector<unsigned char> r(4);
        std::size_t n =
            detail::prepare_greeting(
                r.data(), r.size(), opt);
        r.resize(n);
        return r;
    }

    static
    std::vector<unsigned char>
    make_userpass_request(
        auth_options const& opt = auth_options::none{})
    {
        BOOST_ASSERT(opt.is_userpass);
        std::size_t n =
            opt.user.size() + opt.pass.size() + 3;
        std::vector<unsigned char> r(n);
        std::size_t n2 =
            detail::prepare_userpass_request(
                r.data(), r.size(), opt);
        BOOST_TEST(n == n2);
        ignore_unused(n2);
        return r;
    }

    static
    std::vector<unsigned char>
    make_greet_reply(
        auth_method m = auth_method::no_authentication)
    {
        return {{0x05, static_cast<unsigned char>(m)}};
    }

    static
    std::vector<unsigned char>
    make_request(
        endpoint const& ep = {
            asio::ip::make_address_v4(
                asio::ip::address_v4::uint_type(0)),
                0})
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
    std::vector<unsigned char>
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
    std::vector<unsigned char>
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
    std::vector<unsigned char>
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
    std::vector<unsigned char>
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
    std::vector<unsigned char>
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
        ConstBufferSequence const& requests,
        ConstBufferSequence const& replies,
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
        test::stream s(ioc, fail_at, fail_with);
        s.reset_read(replies);
        error_code ec;
        endpoint app_ep = connect(s, ep, auth, ec);
        BOOST_TEST(s.equal_write_buffers(requests));
        BOOST_TEST_EQ(app_ep, ep);
        BOOST_TEST_EQ(ec, exp_ec);
    };

    static
    void
    checkEndpoint(
        std::initializer_list<
            std::vector<unsigned char>> const& request,
        std::initializer_list<
            std::vector<unsigned char>> const& reply,
        auth_options const& auth,
        error_code exp_ec,
        std::size_t fail_at = 0,
        error_code fail_with = {},
        endpoint const& ep = {
            asio::ip::make_address_v4(
                asio::ip::address_v4::uint_type(0)),
                0})
    {
        std::vector<asio::const_buffer> request_buf;
        for (const auto &r: request)
            request_buf.emplace_back(asio::buffer(r));

        std::vector<asio::const_buffer> reply_buf;
        for (const auto &r: reply)
            reply_buf.emplace_back(asio::buffer(r));
        checkEndpoint(
            request_buf,
            reply_buf,
            auth,
            exp_ec,
            fail_at,
            fail_with,
            ep
        );
    }

    static
    void
    testEndpoint()
    {
        // no auth
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {make_greeting(), make_request()},
                {make_greet_reply(), make_reply()},
                auth_options::none{},
                error::succeeded);
        }

        // user
        {
            auth_options a =
                auth_options::userpass{
                    "user", "pass"};
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {
                    make_greeting(a),
                    make_userpass_request(a),
                    make_request()
                },
                {
                    make_greet_reply(
                        auth_method::userpass),
                    {0x01, 0x00},
                    make_reply()
                },
                a,
                error::succeeded);
        }

        // user (failed to write request)
        {
            auth_options a =
                auth_options::userpass{
                    "user", "pass"};
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {
                    make_greeting(a),
                    make_userpass_request(a)
                },
                {
                    make_greet_reply(
                        auth_method::userpass)
                },
                a,
                error::general_failure,
                2,
                error::general_failure);
        }

        // user (userpass reply failure)
        {
            auth_options a =
                auth_options::userpass{
                    "user", "pass"};
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {
                    make_greeting(a),
                    make_userpass_request(a)
                },
                {
                    make_greet_reply(
                        auth_method::userpass),
                    {0x01, 0x00},
                    make_reply()
                },
                a,
                error::general_failure,
                3,
                error::general_failure);
        }

        // user (bad reply size)
        {
            auth_options a =
                auth_options::userpass{
                    "user", "pass"};
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {
                    make_greeting(a),
                    make_userpass_request(a)
                },
                {
                    make_greet_reply(
                        auth_method::userpass),
                    {0x01}
                },
                a,
                error::bad_reply_size);
        }

        // user (bad reply version)
        {
            auth_options a =
                auth_options::userpass{
                    "user", "pass"};
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {
                    make_greeting(a),
                    make_userpass_request(a)
                },
                {
                    make_greet_reply(
                        auth_method::userpass),
                    {0x05, 0x00}
                },
                a,
                error::bad_reply_version);
        }

        // user (access denied)
        {
            auth_options a =
                auth_options::userpass{
                    "user", "pass"};
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {
                    make_greeting(a),
                    make_userpass_request(a)
                },
                {
                    make_greet_reply(
                        auth_method::userpass),
                    {0x01, 0xFF}
                },
                a,
                error::access_denied);
        }

        // successful ipv6
        {
            asio::ip::address_v6::bytes_type bytes;
            bytes.fill(0x00);
            endpoint ep(asio::ip::make_address_v6(bytes), 0);
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {make_greeting(), make_request(ep)},
                {make_greet_reply(), make_reply_ipv6()},
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
                {make_greeting(), make_request()},
                {make_greet_reply(), reply},
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
                {make_greeting(), make_request()},
                {make_greet_reply(), reply},
                auth_options::none{},
                error::bad_reply_size
            );
        }

        // wrong version
        {
            auto g = make_greet_reply();
            g[0] = 0x04;
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {make_greeting()},
                {g},
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
                {make_greeting()},
                {g},
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
                {make_greeting()},
                {{0x05}},
                auth_options::none{},
                error::bad_reply_size
            );
        }

        // incomplete ipv4 reply
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {make_greeting(), make_request()},
                {make_greet_reply(), make_reply_incomplete()},
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
                {make_greeting(), make_request()},
                {make_greet_reply(), make_reply_ipv6_incomplete()},
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
                {make_greeting(), make_request()},
                {make_greet_reply(), make_reply_unknown()},
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
                {make_greeting(), make_request()},
                {make_greet_reply(), make_reply(static_cast<reply_code>(0xEF))},
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
                {make_greeting(), make_request()},
                {make_greet_reply(), {0x05}},
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
                {make_greeting(), make_request()},
                {make_greet_reply(), r},
                auth_options::none{},
                error::bad_reply_version);
        }

        // request rejected
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {make_greeting(), make_request()},
                {
                    make_greet_reply(),
                    make_reply(
                        reply_code::general_failure)
                },
                auth_options::none{},
                error::general_failure);
        }

        // missing endpoint
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {make_greeting(), make_request()},
                {
                    make_greet_reply(),
                    {
                        0x05,
                        static_cast<unsigned char>(
                            reply_code::succeeded)
                    }
                },
                auth_options::none{},
                error::bad_reply_size);
        }

        // write failure
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {make_greeting()},
                {{}, {}},
                auth_options::none{},
                error::general_failure,
                0,
                error::general_failure);
        }

        // read failure
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {make_greeting()},
                {{}, {}},
                auth_options::none{},
                error::general_failure,
                1,
                error::general_failure);
        }

        // 2nd write failure
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {make_greeting(), make_request()},
                {make_greet_reply(), {}},
                auth_options::none{},
                error::general_failure,
                2,
                error::general_failure);
        }

        // 2nd read failure
        {
            BOOST_TEST_CHECKPOINT();
            checkEndpoint(
                {make_greeting(), make_request()},
                {make_greet_reply(), make_reply()},
                auth_options::none{},
                error::general_failure,
                3,
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
        ConstBufferSequence const& requests,
        ConstBufferSequence const& replies,
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
        s.reset_read(replies);
        // Connect to proxy server
        async_connect(s, ep, auth,
            [&](error_code ec, endpoint app_ep)
        {
            // Compare to expected write values
            BOOST_TEST(s.equal_write_buffers(requests));
            BOOST_TEST_EQ(app_ep, ep);
            BOOST_TEST_EQ(ec, exp_ec);
        });
        ioc.run();
    }

    static
    void
    checkAsyncEndpoint(
        std::initializer_list<
            std::vector<unsigned char>> const& request,
        std::initializer_list<
            std::vector<unsigned char>> const& reply,
        auth_options const& auth,
        error_code exp_ec,
        std::size_t fail_at = 0,
        error_code fail_with = {},
        endpoint const& ep = {
            asio::ip::make_address_v4(
                asio::ip::address_v4::uint_type(0)),
                0})
    {
        std::vector<asio::const_buffer> request_buf;
        for (const auto &r: request)
            request_buf.emplace_back(asio::buffer(r));

        std::vector<asio::const_buffer> reply_buf;
        for (const auto &r: reply)
            reply_buf.emplace_back(asio::buffer(r));
        checkAsyncEndpoint(
            request_buf,
            reply_buf,
            auth,
            exp_ec,
            fail_at,
            fail_with,
            ep
        );
    }

    static
    void
    testAsyncEndpoint()
    {
        // no auth
        {
//            BOOST_TEST_CHECKPOINT();
//            checkAsyncEndpoint(
//                {make_greeting(), make_request()},
//                {make_greet_reply(), make_reply()},
//                auth_options::none{},
//                error::succeeded);
        }

        // user
        {
            auth_options a =
                auth_options::userpass{
                    "user", "pass"};
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                {
                    make_greeting(a),
                    make_userpass_request(a),
                    make_request()
                },
                {
                    make_greet_reply(
                        auth_method::userpass),
                    {0x01, 0x00},
                    make_reply()
                },
                a,
                error::succeeded);
        }

        // user (failed to write request)
        {
            auth_options a =
                auth_options::userpass{
                    "user", "pass"};
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                {
                    make_greeting(a),
                    make_userpass_request(a)
                },
                {
                    make_greet_reply(
                        auth_method::userpass)
                },
                a,
                error::general_failure,
                2,
                error::general_failure);
        }

        // user (failed to read request)
        {
            auth_options a =
                auth_options::userpass{
                    "user", "pass"};
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                {
                    make_greeting(a),
                    make_userpass_request(a)
                },
                {
                    make_greet_reply(
                        auth_method::userpass),
                    {0x01, 0x00}
                },
                a,
                error::general_failure,
                3,
                error::general_failure);
        }

        // reply buf too small
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                {make_greeting(), make_request()},
                {make_greet_reply(), {0x05}},
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
                {make_greeting(), make_request()},
                {make_greet_reply(), r},
                auth_options::none{},
                error::bad_reply_version);
        }

        // request rejected
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                {make_greeting(), make_request()},
                {
                    make_greet_reply(),
                    make_reply(
                      reply_code::
                          connection_refused)
                },
                auth_options::none{},
                error::connection_refused);
        }

        // missing reply endpoint
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                {make_greeting(), make_request()},
                {
                    make_greet_reply(),
                    {{
                        0x05, // VER
                        static_cast<unsigned char>(
                            reply_code::succeeded), // REP
                        0x00 // RSV
                    }}
                },
                auth_options::none{},
                error::bad_reply_size);
        }

        // write failure
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                {make_greeting()},
                {{}},
                auth_options::none{},
                error::general_failure,
                0,
                error::general_failure);
        }

        // read failure
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                {make_greeting()},
                {make_greet_reply()},
                auth_options::none{},
                error::general_failure,
                1,
                error::general_failure);
        }

        // 2nd write failure
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                {make_greeting(), make_request()},
                {make_greet_reply()},
                auth_options::none{},
                error::general_failure,
                2,
                error::general_failure);
        }

        // 2nd read failure
        {
            BOOST_TEST_CHECKPOINT();
            checkAsyncEndpoint(
                {make_greeting(), make_request()},
                {make_greet_reply(), make_reply()},
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
