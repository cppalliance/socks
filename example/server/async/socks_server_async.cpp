//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

//[example_socks_server_async

#include <boost/asio/signal_set.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include <boost/core/detail/string_view.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <memory>
#include <utility>

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;
using endpoint = boost::asio::ip::tcp::endpoint;
using error_code = boost::system::error_code;

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

class socks_connection
    : public std::enable_shared_from_this<socks_connection>
{
public:
    using pointer = std::shared_ptr<socks_connection>;

    static
    pointer
    create(boost::asio::io_context& io_context)
    {
        return pointer(new socks_connection(io_context));
    }

    tcp::socket&
    socket()
    {
        return socket_;
    }

    void
    start()
    {
        std::cout <<
            "Client " << socket_.remote_endpoint() << "\n";
        auto self(shared_from_this());
        /*
         * Read a
         * - SOCKS5 greeting, or
         * - SOCKS4 connect
         */
        asio::async_read(
            socket_,
            asio::buffer(buffer_),
            [this](const error_code& ec, std::size_t n)
            {
                // Stop reading at invalid version
                if (ec.failed()
                    || (n > 0 &&
                        buffer_[0] != 0x05
                        && buffer_[0] != 0x04))
                    return std::size_t(0);

                // SOCKS4 Connect
                // - VER | CMD | DSTPORT | DSTIP | USERID | NULLs
                if (buffer_[0] == 0x04)
                {
                    // USERID ends with NULL
                    // We read from until we find a NULL,
                    // with a limit of buffer_.size()
                    // which means a username of
                    // up to 250 chars
                    const std::size_t maxn = buffer_.size();
                    // Check CMD
                    if (n >= 1 && buffer_[1] != 0x01)
                        return std::size_t(0);
                    // Anything is valid for the next
                    // 6 bytes DSTPORT | DSTIP, so we
                    // look for NULL after buffer_[8]
                    // to stop reading
                    if (n >= 9)
                        for (std::size_t i = 8; i < n; ++i)
                            if (buffer_[i] == 0x00)
                                return std::size_t(0);
                    // Keep allowing the max otherwise
                    return maxn - n;
                }

                // SOCKS5 Greeting
                // - VER / NMETHODS / METHODS
                // Read all methods
                if (n >= 2)
                {
                    std::size_t nmethods = buffer_[1];
                    std::size_t greet_n = 2 + nmethods;
                    if (n < greet_n)
                        return greet_n - n;
                    return std::size_t(0);
                }
                // If n < 2, we can read up to the
                // max number of bytes on next
                // read, assuming a greeting with
                // 256 methods
                return std::size_t(258) - n;
            },
            [this, self](error_code ec, std::size_t n)
            {
                if (!ec.failed())
                {
                    buffer_.resize(n);
                    if (buffer_[0] == 0x05)
                        return parse_socks_greeting();
                    else if (buffer_[0] == 0x04)
                        return parse_socks_connect_v4();
                }
                fail(ec, "Failure to read SOCKS greeting");
            }
        );
    }

private:
    explicit
    socks_connection(asio::io_context& ioc)
        : ioc_(ioc),
          socket_(ioc),
          buffer_(258, 0x00),
          target_socket_(ioc),
          resolver_(ioc)
    {
    }

    void
    parse_socks_greeting()
    {
        // VER / NMETHODS / METHODS
        if (buffer_.empty())
            return fail(
                asio::error::message_size,
                "Empty SOCKS greeting");
        if (buffer_[0] != 0x05)
            return fail(
                asio::error::no_protocol_option,
                "Bad SOCKS version");
        if (buffer_.size() == 1)
            return fail(
                asio::error::message_size,
                "Empty SOCKS auth methods");
        if (buffer_[1] != buffer_.size() - 2)
            return fail(
                asio::error::message_size,
                "Missing SOCKS auth methods");

        // At this point, the range [buffer[2], ...]
        // contains the possible authentication methods
        // We attempt 0x02 (userpass) or 0x00 (no auth)
        for (std::size_t i = 2; i < buffer_.size(); ++i)
        {
            if (buffer_[i] == 0x02)
            {
                server_choice_ = 0x02;
                return do_server_choice();
            }
        }

        for (std::size_t i = 2; i < buffer_.size(); ++i)
        {
            if (buffer_[i] == 0x00)
            {
                server_choice_ = 0x00;
                return do_server_choice();
            }
        }

        // No acceptable method
        server_choice_ = 0xFF;
        do_server_choice();
    }

    void
    do_server_choice()
    {
        buffer_ = {0x05, server_choice_};
        auto self(shared_from_this());
        asio::async_write(
            socket_,
            asio::buffer(buffer_),
            [this, self](error_code ec, std::size_t)
            {
                if (ec.failed())
                    fail(ec, "Cannot write greeting response");
                else if (self->server_choice_ == 0xFF)
                    fail(ec, "No compatible client auth method");
                else if (self->server_choice_ == 0x02)
                    do_userpass_greet();
                else
                    do_read_connect_request();
            }
        );
    }

    void
    do_userpass_greet()
    {
        buffer_.resize(513);
        auto self(shared_from_this());
        asio::async_read(
            socket_,
            asio::buffer(buffer_),
            [this](const error_code& ec, std::size_t n)
            {
                if (ec.failed())
                    return std::size_t(0);
                // Userpass greeting
                constexpr std::size_t nmax = 513;
                // VER | IDLEN | ID  | PWLEN | PW
                // Stop reading at invalid version
                if (n >= 1 && buffer_[0] != 0x01)
                    return std::size_t(0);
                // Read at most nmax when we don't
                // know the IDLEN
                if (n < 2)
                    return nmax;
                std::size_t idlen = buffer_[1];
                std::size_t pwlen = 256;
                if (n > 2 + idlen)
                    pwlen = buffer_[2+idlen];
                return 3 + idlen + pwlen;
            },
            [this, self](error_code ec, std::size_t n)
            {
                if (!ec.failed())
                {
                    buffer_.resize(n);
                    parse_userpass_greeting();
                }
                else
                {
                    fail(ec, "Failure to read SOCKS userpass greeting");
                }
            }
        );
    }

    void
    parse_userpass_greeting()
    {
        // VER | IDLEN | ID  | PWLEN | PW
        if (buffer_.empty())
            return fail(
                asio::error::message_size,
                "Empty userpass greeting");
        if (buffer_[0] != 0x01)
            return fail(
                asio::error::no_protocol_option,
                "Bad userpass version");
        if (buffer_.size() == 1)
            return fail(
                asio::error::message_size,
                "Missing SOCKS username");
        std::size_t idlen = buffer_[1];
        if (buffer_.size() < idlen + 2)
            return fail(
                asio::error::message_size,
                "Incomplete SOCKS username");
        if (buffer_.size() < idlen + 3)
            return fail(
                asio::error::message_size,
                "Missing SOCKS password");
        std::size_t passlen = buffer_[2 + idlen];
        if (buffer_.size() < idlen + 3 + passlen)
            return fail(
                asio::error::message_size,
                "Incomplete SOCKS password");

        // Message is OK
        boost::core::string_view username(
            reinterpret_cast<char*>(buffer_.data()) + 2,
            idlen);
        boost::core::string_view password(
            reinterpret_cast<char*>(buffer_.data()) + 2 + idlen,
            passlen);
        std::cout <<
            "Username: " << username <<
            "Password: " << password << "\n";

        // This server implementation accepts
        // any username and password, so we send
        // a successful reply
        // VER | STATUS
        buffer_ = {0x01, 0x00};
        auto self(shared_from_this());
        asio::async_write(
            socket_,
            asio::buffer(buffer_),
            [this, self](error_code ec, std::size_t)
            {
                if (ec.failed())
                    fail(ec, "Cannot write userpass response");
                else
                    do_read_connect_request();
            }
        );
    }

    void
    do_read_connect_request()
    {
        buffer_.resize(263);
        auto self(shared_from_this());
        asio::async_read(
            socket_,
            asio::buffer(buffer_),
            [this](const error_code& ec, std::size_t n)
            {
                // Greeting
                // VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT
                constexpr std::size_t nmax = 263;
                // Stop reading at invalid VER,
                // CMD, or RSV.
                if (ec.failed()
                    || (n >= 1 && buffer_[0] != 0x05)
                    || (n >= 2 && buffer_[1] != 0x01)
                    || (n >= 3 && buffer_[2] != 0x00))
                    return std::size_t(0);
                if (n < 4)
                    return nmax - n;
                std::size_t req_n;
                switch (buffer_[3])
                {
                case 0x01:
                    req_n = 10;
                    break;
                case 0x03:
                {
                    std::size_t addr_n = 256;
                    if (n > 4)
                        addr_n = buffer_[4];
                    req_n = 7 + addr_n;
                    break;
                }
                case 0x04:
                    req_n = 22;
                    break;
                default:
                    req_n = 0;
                    break;
                }
                if (n < req_n)
                    return req_n - n;
                return std::size_t(0);
            },
            [this, self](error_code ec, std::size_t n)
            {
                if (!ec.failed())
                {
                    buffer_.resize(n);
                    return parse_socks_connect();
                }
                fail(ec, "Failure to read SOCKS connect");
            }
        );
    }

    void
    parse_socks_connect()
    {
        // VER | IDLEN | ID  | PWLEN | PW
        if (buffer_.empty())
            return fail(
                asio::error::message_size,
                "Empty SOCKS connect");
        if (buffer_[0] != 0x05)
        {
            fail(
                asio::error::no_protocol_option,
                "Bad SOCKS request version");
            // connection not allowed by ruleset
            return do_connect_reply(0x02);
        }
        if (buffer_.size() == 1)
        {
            fail(
                asio::error::message_size,
                "Missing SOCKS command");
            // connection not allowed by ruleset
            return do_connect_reply(0x02);
        }
        if (buffer_[1] != 0x01)
        {
            fail(
                asio::error::message_size,
                "Invalid SOCKS connect command");
            // Command not supported
            return do_connect_reply(0x07);
        }
        if (buffer_.size() == 2)
        {
            fail(
                asio::error::message_size,
                "Missing SOCKS reserved byte");
            // connection not allowed by ruleset
            return do_connect_reply(0x02);
        }
        if (buffer_[2] != 0x00)
        {
            fail(
                asio::error::message_size,
                "Invalid SOCKS reserved byte");
            // connection not allowed by ruleset
            return do_connect_reply(0x02);
        }
        if (buffer_.size() == 3)
        {
            fail(
                asio::error::message_size,
                "Missing SOCKS address type");
            // connection not allowed by ruleset
            return do_connect_reply(0x02);
        }
        if (buffer_[3] != 0x01
            && buffer_[3] != 0x03
            && buffer_[3] != 0x04)
        {
            fail(
                asio::error::message_size,
                "Invalid SOCKS address type");
            // Address type not supported
            return do_connect_reply(0x08);
        }
        if (buffer_[3] == 0x01)
        {
            if (buffer_.size() == 10)
            {
                asio::ip::address_v4::bytes_type addr;
                addr[0] = buffer_[4];
                addr[1] = buffer_[5];
                addr[2] = buffer_[6];
                addr[3] = buffer_[7];
                auto ip = ip::make_address_v4(addr);
                std::uint16_t port = 0;
                port |= buffer_[8];
                port <<= 8;
                port |= buffer_[9];
                target_ = {ip, port};
                do_target_connect();
            }
            else
            {
                fail(
                    asio::error::message_size,
                    "Invalid SOCKS ipv4 address");
                // Address type not supported
                return do_connect_reply(0x08);
            }
        }
        else if (buffer_[3] == 0x04)
        {
            if (buffer_.size() == 22)
            {
                ip::address_v6::bytes_type addr;
                for (std::size_t i = 0; i < 16; ++i)
                    addr[i] = buffer_[4 + i];
                auto ip = ip::make_address_v6(addr);
                std::uint16_t port = 0;
                port |= buffer_[20];
                port <<= 8;
                port |= buffer_[21];
                target_ = {ip, port};
                do_target_connect();
            }
            else
            {
                fail(
                    asio::error::message_size,
                    "Invalid SOCKS ipv6 address");
                // Address type not supported
                return do_connect_reply(0x08);
            }
        }
        else if (buffer_[3] == 0x03)
        {
            if (buffer_.size() == std::size_t(7) + buffer_[4])
            {
                std::string host(
                    buffer_.data() + 5,
                    buffer_.data() + 5 + buffer_[4]);
                std::uint16_t port = buffer_[buffer_.size() - 2];
                port <<= 8;
                port |= buffer_[buffer_.size() - 1];
                std::string service = to_string(port);
                auto self(shared_from_this());
                resolver_.async_resolve(
                    host,
                    service,
                    [this, self](
                        error_code ec,
                        tcp::resolver::results_type eps)
                    {
                        if (ec.failed())
                        {
                            std::string host(
                                buffer_.data() + 5,
                                buffer_.data() + 5 + buffer_[4]);
                            fail(
                                ec,
                                "Cannot resolve domain name",
                                host);
                            // Host unreachable
                            return do_connect_reply(0x04);
                        }
                        target_ = eps.begin()->endpoint();
                        do_target_connect();
                    }
                );
            }
            else
            {
                fail(asio::error::message_size,
                     "Invalid SOCKS domain address");
                // Address type not supported
                do_connect_reply(0x08);
            }
        }
    }

    void
    parse_socks_connect_v4()
    {
        // VER | CMD | DSTPORT | DSTIP | USERID | NULL
        if (buffer_.empty())
            return fail(
                asio::error::message_size,
                "Empty SOCKS connect");
        if (buffer_[0] != 0x04)
        {
            fail(
                asio::error::no_protocol_option,
                "Bad request version");
            // request rejected or failed
            return do_connect_reply_v4(91);
        }
        if (buffer_.size() == 1)
        {
            fail(
                asio::error::message_size,
                "Missing SOCKS command");
            // request rejected or failed
            return do_connect_reply_v4(91);
        }
        if (buffer_[1] != 0x01)
        {
            fail(
                asio::error::message_size,
                "Invalid SOCKS connect command");
            // request rejected or failed
            return do_connect_reply_v4(91);
        }
        if (buffer_.size() >= 8)
        {
            std::uint16_t port = 0;
            port |= buffer_[2];
            port <<= 8;
            port |= buffer_[3];
            asio::ip::address_v4::bytes_type addr;
            addr[0] = buffer_[4];
            addr[1] = buffer_[5];
            addr[2] = buffer_[6];
            addr[3] = buffer_[7];
            auto ip = ip::make_address_v4(addr);
            target_ = {ip, port};
            /*
             * [buffer[8], ...] contain the
             * ident id, which this server
             * ignores
             */
            return do_target_connect_v4();
        }
        else
        {
            fail(
                asio::error::message_size,
                "Invalid SOCKS ipv4 address");
            // request rejected or failed
                return do_connect_reply_v4(91);
        }
    }

    void
    do_target_connect()
    {
        auto self(shared_from_this());
        target_socket_.async_connect(
            target_,
            [this, self](error_code ec)
            {
                if (ec.failed())
                {
                    fail(
                        ec,
                        "Cannot connect to target",
                        target_);
                    // Host unreachable
                    return do_connect_reply(0x04);
                }
                // succeeded
                do_connect_reply(0x00);
            }
        );
    }

    void
    do_target_connect_v4()
    {
        auto self(shared_from_this());
        target_socket_.async_connect(
            target_,
            [this, self](error_code ec)
            {
                if (ec.failed())
                {
                    fail(
                        ec,
                        "Cannot connect to target",
                        target_);
                    // request rejected or failed
                    return do_connect_reply_v4(91);
                }
                // request granted
                return do_connect_reply_v4(90);
            }
        );
    }

    void
    do_connect_reply(unsigned char rep)
    {
        endpoint bnd_ep =
            target_socket_.local_endpoint();
        if (rep != 0x00)
        {
            buffer_.resize(10);
            // VER
            buffer_[0] = 0x05;
            // REP
            buffer_[1] = rep;
            // RSV
            buffer_[2] = 0x00;
            // ATYP
            buffer_[3] = 0x01;
            // DST.ADDR
            buffer_[4] = 0x00;
            buffer_[5] = 0x00;
            buffer_[6] = 0x00;
            buffer_[7] = 0x00;
            // DST.PORT
            buffer_[8] = 0x00;
            buffer_[9] = 0x00;
        }
        else if (bnd_ep.address().is_v4())
        {
            buffer_.resize(10);
            // VER
            buffer_[0] = 0x05;
            // REP
            buffer_[1] = rep;
            // RSV
            buffer_[2] = 0x00;
            // ATYP
            buffer_[3] = 0x01;
            // DST.ADDR
            auto bnd_v4 = bnd_ep.address().to_v4();
            auto bnd_bytes = bnd_v4.to_bytes();
            buffer_[4] = bnd_bytes[0];
            buffer_[5] = bnd_bytes[1];
            buffer_[6] = bnd_bytes[2];
            buffer_[7] = bnd_bytes[3];
            // DST.PORT
            std::uint16_t p = bnd_ep.port();
            buffer_[8] = (p >> 8) & 0xFF;
            buffer_[9] = p & 0xFF;
        }
        else /* if (bnd_ep.address().is_v6()) */
        {
            buffer_.resize(22);
            // VER
            buffer_[0] = 0x05;
            // REP
            buffer_[1] = rep;
            // RSV
            buffer_[2] = 0x00;
            // ATYP
            buffer_[3] = 0x04;
            // DST.ADDR
            auto bnd_v6 = bnd_ep.address().to_v6();
            auto bnd_bytes = bnd_v6.to_bytes();
            for (std::size_t i = 0; i < 16; ++i)
                buffer_[i + 4] = bnd_bytes[i];
            // DST.PORT
            std::uint16_t p = bnd_ep.port();
            buffer_[20] = (p >> 8) & 0xFF;
            buffer_[21] = p & 0xFF;
        }
        auto self(shared_from_this());
        asio::async_write(
            socket_,
            asio::buffer(buffer_),
            [this, self](error_code ec, std::size_t)
            {
                if (ec.failed())
                    return fail(
                        ec, "Cannot write connect reply");
                unsigned char rep = buffer_[1];
                if (rep == 0x00)
                    do_relay();
                else
                    socket_.close();
            }
        );
    }

    void
    do_connect_reply_v4(unsigned char rep)
    {
        endpoint bnd_ep =
            target_socket_.local_endpoint();
        buffer_.resize(8);
        // VER
        buffer_[0] = 0x04;
        // REP
        buffer_[1] = rep;
        if (rep != 90)
        {
            // DST.PORT
            buffer_[2] = 0x00;
            buffer_[3] = 0x00;
            // DST.ADDR
            buffer_[4] = 0x00;
            buffer_[5] = 0x00;
            buffer_[6] = 0x00;
            buffer_[7] = 0x00;
        }
        else
        {
            // DST.PORT
            std::uint16_t p = bnd_ep.port();
            buffer_[2] = (p >> 8) & 0xFF;
            buffer_[3] = p & 0xFF;
            // DST.ADDR
            if (bnd_ep.address().is_v4())
            {
                auto bnd_v4 = bnd_ep.address().to_v4();
                auto bnd_bytes = bnd_v4.to_bytes();
                buffer_[4] = bnd_bytes[0];
                buffer_[5] = bnd_bytes[1];
                buffer_[6] = bnd_bytes[2];
                buffer_[7] = bnd_bytes[3];
            }
            else
            {
                buffer_[4] = 0x00;
                buffer_[5] = 0x00;
                buffer_[6] = 0x00;
                buffer_[7] = 0x00;
            }
        }
        auto self(shared_from_this());
        asio::async_write(
            socket_,
            asio::buffer(buffer_),
            [this, self](error_code ec, std::size_t)
            {
                if (ec.failed())
                    return fail(
                        ec, "Cannot write connect reply");
                unsigned char v = buffer_[0];
                unsigned char rep = buffer_[1];
                if ((v == 0x05
                     && rep == 0x00)
                    || (v == 0x04
                        && rep == 90))
                    do_relay();
                else
                    socket_.close();
            }
        );
    }

    void
    do_relay()
    {
        // Relay anything from client to target
        // and vice-versa
        do_relay_from_client();
        do_relay_from_target();
    }

    void
    do_relay_from_client()
    {
        buffer_.resize(1024);
        auto self(shared_from_this());
        socket_.async_read_some(
            asio::buffer(buffer_),
            [this, self](error_code ec, std::size_t n)
            {
                if (ec == asio::error::eof)
                    return fail(ec, "Client disconnected");
                else if (ec.failed())
                    return fail(ec, "Cannot read from client");
                buffer_.resize(n);
                do_relay_to_target();
            }
        );
    }

    void
    do_relay_to_target()
    {
        auto self(shared_from_this());
        target_socket_.async_write_some(
            asio::buffer(buffer_),
            [this, self](error_code ec, std::size_t)
            {
                if (ec.failed())
                    return fail(ec, "Cannot write to target");
                do_relay_from_client();
            }
        );
    }

    void
    do_relay_from_target()
    {
        target_buffer_.resize(1024);
        auto self(shared_from_this());
        target_socket_.async_read_some(
            asio::buffer(target_buffer_),
            [this, self](error_code ec, std::size_t n)
            {
                if (ec.failed())
                    return fail(ec, "Application server disconnected");
                else if (ec.failed())
                    return fail(ec, "Cannot read from target");
                target_buffer_.resize(n);
                do_relay_to_client();
            }
        );
    }

    void
    do_relay_to_client()
    {
        auto self(shared_from_this());
        socket_.async_write_some(
            asio::buffer(target_buffer_),
            [this, self](error_code ec, std::size_t)
            {
                if (ec.failed())
                    return fail(ec, "Cannot write to client");
                do_relay_from_target();
            }
        );
    }

    static
    void
    fail(error_code ec, char const* what)
    {
        std::cerr << what << ": " << ec.message() << "\n";
    }

    template <class T>
    static
    void
    fail(error_code ec, char const* what, T const& t)
    {
        std::cerr << what << ": " << ec.message() << ": " << t << "\n";
    }

    asio::io_context& ioc_;
    tcp::socket socket_;
    std::vector<unsigned char> buffer_;
    std::vector<unsigned char> target_buffer_;
    unsigned char server_choice_{0x00};
    endpoint target_{};
    tcp::socket target_socket_;
    tcp::resolver resolver_;
};

class socks_server
{
public:
    socks_server(
        asio::io_context& io_context,
        std::string const& listen_address,
        std::string const& listen_port)
        : ioc_(io_context),
          listen_endpoint_(*tcp::resolver(ioc_).resolve(
            listen_address,
            listen_port,
            tcp::resolver::passive)),
          acceptor_(io_context, listen_endpoint_)
    {
        std::cout << "Listening on " << listen_endpoint_ << "\n";
        do_accept();
    }

private:
    void do_accept()
    {
        socks_connection::pointer new_connection =
            socks_connection::create(ioc_);
        acceptor_.async_accept(
            new_connection->socket(),
            [this, new_connection](
                const boost::system::error_code& ec) {
                if (!ec.failed())
                    new_connection->start();
                do_accept();
            });
    }

    boost::asio::io_context& ioc_;
    tcp::endpoint listen_endpoint_;
    tcp::acceptor acceptor_;
};


int main(int argc, char** argv)
{
    // Check command line arguments.
    std::string listen_address = "localhost";
    std::string listen_port = "1080";
    if (argc != 3)
    {
        std::cerr <<
            "Usage: socks_server_async <listen address> <listen port>\n\n"
            "Example:\n"
            "    socks_server_async localhost 1080\n"
            "Using default values:\n"
            "    - <listen address>: localhost\n"
            "    - <listen port>: 1080\n\n";
        if (argc >= 2)
            listen_address = argv[1];
        if (argc > 3)
            listen_port = argv[2];
    }
    else
    {
        listen_address = argv[1];
        listen_port = argv[2];
    }

    try
    {
        asio::io_context ioc;
        asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait(
            [&ioc](error_code const&, std::size_t)
            {
                ioc.stop();
            }
        );
        socks_server server(
            ioc, listen_address, listen_port);
        ioc.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return EXIT_SUCCESS;
}

//]
