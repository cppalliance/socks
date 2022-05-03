//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <boost/socks_proto/connect.hpp>
#include <boost/socks_proto/connect_v4.hpp>

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
using string_view = socks::string_view;
using error_code = socks::error_code;
using endpoint = socks::endpoint;

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
        if (ec_.failed())
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
        if (socks_.host_type() == urls::host_type::name)
        {
            // Look up the SOCKS domain name
            resolver_.async_resolve(
                std::string(socks_.encoded_host()),
                std::to_string(default_port(socks_)),
                [this](
                    error_code ec,
                    tcp::resolver::results_type endpoints) {
                if (ec.failed())
                    return fail(ec, "proxy resolve");
                do_proxy_connect(
                    endpoints.begin(), endpoints.end());
            });
        }
        else if (socks_.host_type() == urls::host_type::ipv4 ||
                 socks_.host_type() == urls::host_type::ipv6)
        {
            // Nothing to resolve
            tcp::endpoint ep = get_endpoint_unchecked(socks_);
            do_proxy_connect(&ep, (&ep)+1);
        }
        else
        {
            // Cannot interpret the host
            fail(asio::error::operation_not_supported,
                 "Unsupported host",
                 socks_.encoded_host());
        }
    }

    template <class EndpointIt>
    void
    do_proxy_connect(EndpointIt first, EndpointIt last)
    {
        asio::async_connect(socket_, first, last,
            [this](error_code ec, EndpointIt) {
            if (ec.failed())
                return fail(ec, "connect");
            do_socks_connect();
        });
    }

    void
    do_socks_connect()
    {
        // The callback function for whatever
        // connect function we use
        auto cb = [this](error_code ec, tcp::endpoint) {
            if (ec.failed())
                return fail(ec, "socks_connect");
            do_socks_request();
        };
        // Send a SOCKS connect request according to the URL
        if (socks_version_ == 0x05)
        {
            if (!socks_.has_userinfo()) {
                if (target_.host_type() == urls::host_type::name)
                {
                    socks::async_connect(
                        socket_,
                        target_.encoded_host(),
                        default_port(target_),
                        socks::auth::no_auth{},
                        cb);
                }
                else if (target_.host_type() == urls::host_type::ipv4 ||
                         target_.host_type() == urls::host_type::ipv6)
                {
                    socks::async_connect(
                        socket_,
                        get_endpoint_unchecked(target_),
                        socks::auth::no_auth{},
                        cb);
                }
            }
            else
            {
                socks::auth::userpass a{
                    socks_.encoded_user(),
                    socks_.encoded_password(),
                };
                if (target_.host_type() == urls::host_type::name)
                {
                    socks::async_connect(
                        socket_,
                        target_.encoded_host(),
                        default_port(target_),
                        a,
                        cb);
                }
                else if (target_.host_type() == urls::host_type::ipv4 ||
                         target_.host_type() == urls::host_type::ipv6)
                {
                    socks::async_connect(
                        socket_,
                        get_endpoint_unchecked(target_),
                        a,
                        cb);
                }
            }
        }
        else
        {
            if (target_.host_type() == urls::host_type::name)
            {
                // SOCKS4 does not support domain names.
                // The domain name needs to be resolved
                // on the client.
                std::size_t port = default_port(target_);
                std::string port_str = to_string(port);
                using resolve_results =
                    asio::ip::tcp::resolver::results_type;
                resolver_.async_resolve(
                    std::string(target_.encoded_host()),
                    port_str,
                    [this, cb]
                    (error_code ec, resolve_results eps) {
                        if (ec.failed())
                            return fail(ec, "resolve");
                        auto it = eps.begin();
                        while (it != eps.end())
                        {
                            auto e = it->endpoint();
                            if (e.address().is_v4())
                            {
                                // Send the CONNECT request
                                socks::async_connect_v4(
                                    socket_,
                                    e,
                                    socks_.encoded_user(),
                                    cb);
                                return;
                            }
                            ++it;
                        }
                        fail(asio::error::host_not_found,
                            "not ipv4 address found for host");
                    });
            }
            else if (target_.host_type() == urls::host_type::ipv4 ||
                     target_.host_type() == urls::host_type::ipv6)
            {
                socks::async_connect_v4(
                    socket_,
                    get_endpoint_unchecked(target_),
                    socks_.encoded_user(),
                    cb);
            }
        }
    }

    void
    do_socks_request()
    {
        /*
         * After this point, we can talk to the
         * SOCKS server as if we were talking to
         * the application server.
         */

        // Set up an HTTP GET request
        req_ = {http::verb::get, target_.encoded_host(), http_version_};
        req_.set(http::field::host, target_.encoded_host());
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        std::cout << req_ << std::endl;

        // Send the HTTP request
        http::async_write(socket_, req_,
            [this](error_code ec, std::size_t) {
            if (ec.failed())
                return fail(ec, "write");
            do_socks_response();
        });
    }

    void do_socks_response()
    {
        // Read the HTTP response
        http::async_read(socket_, buffer_, res_,
             [this](error_code ec, std::size_t) {
            if (ec.failed() &&
                ec != beast::http::error::end_of_stream)
                return fail(ec, "read");

            // Print response
            std::cout << res_ << std::endl;

            // Close the socket
            socket_.shutdown(
                tcp::socket::shutdown_both, ec);
            if (ec.failed() && ec != beast::errc::not_connected)
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
            return fail(r.error(), "Parse target");
        target_ = r.value();

        // Parse SOCKS server URI
        r = urls::parse_uri(socks_str);
        if (!r.has_value())
            return fail(r.error(), "Parse SOCKS");
        socks_ = r.value();
        if (socks_.scheme() == "socks5")
            socks_version_ = 0x05;
        else if (
            socks_.scheme() == "socks4"
            || socks_.scheme() == "socks4a")
            socks_version_ = 0x04;
        else
            return fail(asio::error::no_protocol_option,
                        "Invalid SOCKS scheme: ",
                        socks_.scheme());

        // Validate parameters
        if (socks_version_ == 0x04
            && target_.host_type() == urls::host_type::ipv6)
            return fail(asio::error::no_protocol_option,
                        "SOCKS4 does not support IPv6 addresses");
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

    void
    fail(error_code ec, char const* what, string_view value)
    {
        std::cerr << what <<
            " - " << value <<
            ": " << ec.category().name() <<
            " - " << ec.message() << "\n";
        ec_ = ec;
    }

    tcp::resolver resolver_;
    tcp::socket socket_;
    int http_version_;
    urls::url target_;
    urls::url socks_;
    unsigned char socks_version_{0x04};
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
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