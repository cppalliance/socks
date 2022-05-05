//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <boost/socks_proto/connect.hpp>
#include <boost/socks_proto/connect_v4.hpp>

#include <boost/url/url_view.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

namespace socks = boost::socks_proto;
namespace urls = boost::urls;
namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;
using string_view = socks::string_view;
using endpoint = socks::endpoint;
using error_code = socks::error_code;

//------------------------------------------------------------------------------

// A small version of std::to_string to avoid a
// common bug on MinGw.
std::string
to_string(std::uint16_t v)
{
#if (defined(__MINGW32__) || defined(MINGW32) || defined(BOOST_MINGW32))
    constexpr int bn = 4 * sizeof(std::uint16_t);
    char str[bn];
    int n = std::snprintf(str, bn, "%d", v);
    BOOST_ASSERT(n <= bn);
    boost::ignore_unused(n);
    return std::string(str);
#else
    return std::to_string(v);
#endif
}

endpoint
connect_v4(
    asio::ip::tcp::socket& stream,
    string_view target_host,
    std::uint16_t target_port,
    string_view socks_user,
    error_code& ec)
{
    // SOCKS4 does not support domain names.
    // The domain name needs to be resolved
    // on the client.
    asio::ip::tcp::resolver resolver{stream.get_executor()};
    asio::ip::tcp::resolver::results_type endpoints =
        resolver.resolve(
            std::string(target_host),
            to_string(target_port),
            ec);
    if (ec.failed())
        return endpoint{};

    auto it = endpoints.begin();
    while (it != endpoints.end())
    {
        endpoint ep = it->endpoint();
        ++it;
        // SOCKS4 does not support IPv6 addresses
        if (ep.address().is_v6())
        {
            if (!ec.failed())
                ec = asio::error::host_not_found;
            continue;
        }
        ep.port(target_port);
        ep = socks::connect_v4(
            stream,
            ep,
            socks_user,
            ec);
        if (ec.failed())
            continue;
        else
            return ep;
    }
    return endpoint{};
}

// Report a failure
bool
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
    return false;
}

bool
fail(char const* what)
{
    std::cerr << what << "\n";
    return false;
}

bool
fail(char const* what, string_view value)
{
    std::cerr << what << ": " << value << "\n";
    return false;
}

std::uint16_t
default_port(const boost::urls::url_view& u)
{
    if (!u.has_port())
    {
        if (u.scheme_id() == boost::urls::scheme::http)
            return 80;
        if (u.scheme_id() == boost::urls::scheme::https)
            return 445;
        if (u.scheme().starts_with("socks"))
            return 1080;
    }
    return u.port_number();
}

tcp::endpoint
get_endpoint_unchecked(const boost::urls::url_view& u)
{
    if (u.host_type() == urls::host_type::ipv4)
    {
        urls::ipv4_address ip =
            urls::parse_ipv4_address(u.encoded_host()).value();
        return tcp::endpoint{
            ip::address_v4{ip.to_uint()},
            default_port(u)};
    }
    urls::ipv6_address ip =
        urls::parse_ipv6_address(u.encoded_host()).value();
    return tcp::endpoint{
        ip::address_v6{ip.to_bytes()},
        default_port(u)};
}

bool
socks_request(
    asio::io_context& ioc,
    int http_version,
    string_view target_str,
    string_view socks_str)
{
    // Parse target application URI
    auto r = urls::parse_uri(target_str);
    if (!r.has_value())
        return fail(r.error(), "Parse target");
    urls::url_view target = r.value();

    // Parse SOCKS server URI
    r = urls::parse_uri(socks_str);
    if (!r.has_value())
        return fail(r.error(), "Parse SOCKS");
    urls::url_view socks = r.value();
    unsigned char socks_version = 0x00;
    if (socks.scheme() == "socks5")
        socks_version = 0x05;
    else if (socks.scheme() == "socks4" ||
             socks.scheme() == "socks4a")
        socks_version = 0x04;
    else
        return fail("Invalid SOCKS scheme: ", socks.scheme());

    // Validate parameters
    if (socks_version == 0x04 &&
        target.host_type() == urls::host_type::ipv6)
        return fail("SOCKS4 does not support IPv6 addresses");

    // Connect to the SOCKS server
    tcp::socket socket{ioc};
    beast::error_code ec;
    if (socks.host_type() == urls::host_type::name)
    {
        tcp::resolver resolver{ioc};
        auto const endpoints = resolver.resolve(
            std::string(socks.encoded_host()),
            std::to_string(default_port(socks)),
            ec);
        if (ec.failed())
            return fail(ec, "resolve");
        asio::connect(socket, endpoints, ec);
    }
    else if (socks.host_type() == urls::host_type::ipv4 ||
             socks.host_type() == urls::host_type::ipv6)
        socket.connect(get_endpoint_unchecked(socks), ec);
    else
        return fail("Unsupported host", socks.encoded_host());

    if (ec.failed())
        return fail(ec, "connect");

    // Send a SOCKS connect request according to the URL
    if (socks_version == 0x05)
    {
        if (!socks.has_userinfo()) {
            if (target.host_type() == urls::host_type::name)
            {
                socks::connect(
                    socket,
                    target.encoded_host(),
                    default_port(target),
                    socks::auth_options::none{},
                    ec);
            }
            else if (target.host_type() == urls::host_type::ipv4 ||
                     target.host_type() == urls::host_type::ipv6)
            {
                socks::connect(
                    socket,
                    get_endpoint_unchecked(target),
                    socks::auth_options::none{},
                    ec);
            }
        }
        else
        {
            socks::auth_options::userpass a{
                socks.encoded_user(),
                socks.encoded_password(),
            };
            if (target.host_type() == urls::host_type::name)
            {
                socks::connect(
                    socket,
                    target.encoded_host(),
                    default_port(target),
                    a,
                    ec);
            }
            else if (target.host_type() == urls::host_type::ipv4 ||
                     target.host_type() == urls::host_type::ipv6)
            {
                socks::connect(
                    socket,
                    get_endpoint_unchecked(target),
                    a,
                    ec);
            }
        }
    }
    else
    {
        if (target.host_type() == urls::host_type::name)
        {
            connect_v4(
                socket,
                target.encoded_host(),
                default_port(target),
                socks.encoded_user(),
                ec);
        }
        else if (target.host_type() == urls::host_type::ipv4 ||
                 target.host_type() == urls::host_type::ipv6)
        {
            socks::connect_v4(
                socket,
                get_endpoint_unchecked(target),
                socks.encoded_user(),
                ec);
        }
    }
    if (ec.failed())
        return fail(ec, "socks_connect");

    /*
     * After this point, we can talk to the
     * SOCKS server as if we were talking to
     * the application server.
     */

    // Set up an HTTP GET request
    http::request<http::string_body> req{
        http::verb::get,
        target.encoded_path(),
        http_version
    };
    req.set(http::field::host, target.encoded_host());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    std::cout << req << std::endl;

    // Send the HTTP request
    http::write(socket, req, ec);
    if (ec.failed())
        return fail(ec, "write");

    // Read the HTTP response
    beast::flat_buffer b;
    http::response<http::dynamic_body> res;
    http::read(socket, b, res, ec);
    if (ec.failed() &&
        ec != beast::http::error::end_of_stream)
        return fail(ec, "read");
    std::cout << res << std::endl;

    // Close the socket
    socket.shutdown(tcp::socket::shutdown_both, ec);
    if(ec.failed() && ec != beast::errc::not_connected)
        return fail(ec, "shutdown");

    // If we get here, then everything went fine
    return true;
}

int main(int argc, char** argv)
{
    // Check command line arguments.
    if(argc != 3)
    {
        std::cerr <<
            "Usage: socks_client_sync <target URL> <socks URL>\n\n"
            "Arguments:\n"
            "    - <target URL>: Application server URI\n"
            "    - <socks URL>: SOCKS server URI (<socks[4|5]://[[user]:password@]server:port>)\n\n"
            "Example:\n"
            "    socks_client_sync http://www.example.com:80/ socks5://socks5server.com:1080\n";
        return EXIT_FAILURE;
    }

    asio::io_context ioc;
    if (!socks_request(ioc, 11, argv[1], argv[2]))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}