//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <boost/socks_proto/version.hpp>
#include <boost/socks_proto/command.hpp>
#include <boost/socks_proto/reply_code.hpp>
#include <boost/socks_proto/reply_code_v4.hpp>

#include <boost/socks_proto/io/connect_v4.hpp>

#include <boost/url/url.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

namespace socks = boost::socks_proto;
namespace urls = boost::urls;
namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;
using string_view = beast::string_view;
using error_code = boost::system::error_code;

//------------------------------------------------------------------------------

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

class socks_client
{
public:
    explicit socks_client(
        asio::io_context& ioc,
        int http_version,
        string_view target_str,
        string_view socks_str)
    : resolver_(ioc)
    , socket_(ioc)
    , http_version_(http_version)
    {
        parse_urls(target_str, socks_str);
        if (ec_)
            return;
        do_proxy_resolve();
    }

    bool
    has_error() const
    {
        return static_cast<bool>(ec_);
    }

    error_code
    error() const
    {
        return ec_;
    }

private:
    void
    do_proxy_resolve()
    {
        // Resolve to the SOCKS server
        urls::host_type t = socks_.host_type();
        if (t == urls::host_type::name)
        {
            // Look up the SOCKS domain name
            resolver_.async_resolve(
                std::string(socks_.encoded_host()),
                std::to_string(default_port(socks_)),
                [this](
                    error_code ec,
                    tcp::resolver::results_type endpoints) {
                if (ec)
                    return fail(ec, "proxy resolve");
                do_proxy_connect(
                    endpoints.begin(), endpoints.end());
            });
        }
        else if (t == urls::host_type::ipv4)
        {
            auto ipr = urls::parse_ipv4_address(
                socks_.encoded_host());
            if (ipr.has_error())
            {
                std::cerr
                    << "Cannot parse IPv4 address "
                    << socks_.encoded_host()
                    << "\n";
                ec_ = asio::error::operation_not_supported;
                return;
            }
            urls::ipv4_address ip_addr{
                ipr.value().to_uint()
            };
            tcp::endpoint ep{
                ip::address_v4{ ip_addr.to_uint() },
                default_port(socks_)
            };
            do_proxy_connect(&ep, (&ep)+1);
        }
        else if (t == urls::host_type::ipv6)
        {
            auto ipr = urls::parse_ipv6_address(
                socks_.encoded_host());
            if (ipr.has_error())
            {
                std::cerr
                    << "Cannot parse IPv6 address "
                    << socks_.encoded_host()
                    << "\n";
                ec_ = asio::error::operation_not_supported;
                return;
            }
            urls::ipv6_address ip_addr{
                ipr.value().to_bytes()
            };
            tcp::endpoint ep{
                ip::address_v6{ ip_addr.to_bytes() },
                default_port(socks_)
            };
            do_proxy_connect(&ep, (&ep)+1);
        }
        else
        {
            std::cerr
                << "Unsupported host "
                << socks_.encoded_host() << "\n";
            ec_ = asio::error::operation_not_supported;
        }
    }

    template <class EndpointIt>
    void
    do_proxy_connect(EndpointIt first, EndpointIt last)
    {
        asio::async_connect(socket_, first, last,
            [this](error_code ec, EndpointIt) {
            if (ec)
                return fail(ec, "connect");
            // Procedure:
            // - Handshake: SOCKS5 only
            // - Authentication: SOCKS5 only
            // - Socks Resolve: SOCKS4 only
            // - Connect: SOCKS4 and SOCKS5
            if (target_.host_type() == urls::host_type::name)
                do_socks_resolve();
            else
                do_socks_connect();
        });
    }

    void
    do_socks_resolve()
    {
        // SOCKS4 does not support domain names
        // The domain name needs to be resolved
        // on the client
        resolver_.async_resolve(
            std::string(target_.encoded_host()),
            std::to_string(default_port(target_)),
            [this](
                error_code ec,
                tcp::resolver::results_type endpoints) {
            if (ec)
                return fail(ec, "socks resolve");
            // SOCKS4 does not support IPv6 addresses
            auto it = endpoints.begin();
            while (it != endpoints.end())
            {
                auto e = it->endpoint();
                if (e.address().is_v4())
                {
                    // update target in SOCKS4
                    // and connect to IPv4
                    urls::ipv4_address ip{
                        e.address().to_v4().to_bytes()};
                    target_.set_host(ip);
                    do_socks_connect();
                    return;
                }
                ++it;
            }
            ec_ = asio::error::host_not_found;
        });
    }

    void
    do_socks_connect()
    {
        // Continue with SOCKS4 procedure, as
        // SOCKS5 is not supported yet.
        // In SOCKS4, we skip authentication
        // and go straight to CONNECT
        tcp::endpoint ep{
            ip::make_address_v4(
                std::string(target_.encoded_host())),
            target_.port_number()
        };
        socks::io::async_connect_v4(
            socket_,
            ep,
            socks_.encoded_userinfo(),
            [this](error_code ec, tcp::endpoint) {
                if (ec)
                    return fail(ec, "socks_connect_v4");
                /*
                 * From this point, we just talk to the
                 * SOCKS server as if we were talking to
                 * the application server.
                 */
                do_socks_request();
            }
        );
    }

    void
    do_socks_request()
    {
        // Set up an HTTP GET request
        http::request<http::string_body> req{
            http::verb::get,
            target_.encoded_host(),
            http_version_
        };
        req.set(
            http::field::host,
            target_.encoded_host());
        req.set(
            http::field::user_agent,
            BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request
        http::async_write(socket_, req,
            [this](error_code ec, std::size_t) {
            if (ec)
                return fail(ec, "write");
            do_socks_response();
        });
    }

    void do_socks_response()
    {
        // Read the HTTP response
        http::async_read(socket_, buffer_, res_,
             [this](error_code ec, std::size_t) {
            if (ec)
                return fail(ec, "read");

            // Print response
            std::cout << res_ << std::endl;

            // Close the socket
            socket_.shutdown(
                tcp::socket::shutdown_both, ec);
            if (ec && ec != beast::errc::not_connected)
                return fail(ec, "shutdown");
        });
    }

    void
    parse_urls(
        string_view target_str,
        string_view socks_str)
    {
        // Parse target application URI
        auto r = urls::parse_uri(target_str);
        if (!r.has_value())
        {
            fail(r.error(), "Parse target");
            return;
        }
        target_ = r.value();

        // Parse SOCKS server URI
        r = urls::parse_uri(socks_str);
        if (!r.has_value())
        {
            fail(r.error(), "Parse SOCKS");
            return;
        }
        socks_ = r.value();
        if (socks_.scheme() == "socks5")
        {
            socks_version_ = socks::version::
                socks_5;
        }
        else if (
            socks_.scheme() == "socks4"
            || socks_.scheme() == "socks4a")
        {
            socks_version_ = socks::version::
                socks_4;
        }
        else
        {
            std::cerr
                << "Invalid SOCKS Scheme:"
                << socks_.scheme()
                << "\n"
                   "Valid schemes: \"socks5\" and \"socks4\"\n";
            ec_ = asio::error::no_protocol_option;
            return;
        }

        // Validate parameters
        if (socks_version_ == socks::version::socks_4
            && target_.host_type() == urls::host_type::ipv6)
        {
            std::cerr
                << "SOCKS4 does not support IPv6 addresses\n";
            ec_ = asio::error::no_protocol_option;
            return;
        }
        else if (
            socks_version_ == socks::version::socks_5)
        {
            std::cerr << "SOCKS5 not supported by this client\n";
            ec_ = asio::error::no_protocol_option;
            return;
        }
    }

    // Report a failure
    void
    fail(error_code ec, char const* what)
    {
        std::cerr << what <<
            ": " << ec.category().name() <<
            " - " << ec.message() << "\n";
        ec_ = ec;
    }

    tcp::resolver resolver_;
    tcp::socket socket_;
    int http_version_;
    urls::url target_;
    urls::url socks_;
    socks::version socks_version_{socks::version::socks_4};
    beast::flat_buffer buffer_;
    http::response<http::string_body> res_;
    error_code ec_;

};

int main(int argc, char** argv)
{
    // Check command line arguments.
    if (argc != 3)
    {
        std::cerr <<
            "Usage: socks_client_async <target URL> <socks URL>\n\n"
            "Arguments:\n"
            "    - <target URL>: Application server URI\n"
            "    - <socks URL>: SOCKS server URI (<socks[4|5]://[[user]:password@]server:port>)\n\n"
            "Example:\n"
            "    socks_client_sync http://www.example.com:80/ socks5://socks5server.com:1080\n";
        return EXIT_FAILURE;
    }

    /*
     * Some SOCKS proxy lists:
     * https://www.proxy-list.download/SOCKS4
     * https://www.proxy-list.download/SOCKS5
     * https://spys.one/en/socks-proxy-list/
     */
    asio::io_context ioc;
    socks_client c(ioc, 11, argv[1], argv[2]);
    ioc.run();
    if (c.has_error())
    {
        std::cerr <<
            "Client on " << argv[2] <<
            " - " << c.error().category().name() <<
            " - " << c.error().message() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}