//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REPLY_CODE_IPP
#define BOOST_HTTP_PROTO_IMPL_REPLY_CODE_IPP

#include <boost/socks_proto/reply_code.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace socks_proto {

reply_code
to_reply_code(unsigned v)
{
    switch(static_cast<reply_code>(v))
    {
    case reply_code::succeeded:
    case reply_code::general_failure:
    case reply_code::connection_not_allowed_by_ruleset:
    case reply_code::network_unreachable:
    case reply_code::host_unreachable:
    case reply_code::connection_refused:
    case reply_code::ttl_expired:
    case reply_code::command_not_supported:
    case reply_code::address_type_not_supported:
        return static_cast<reply_code>(v);
    default:
        break;
    }
    return reply_code::unassigned;
}

string_view
to_string(reply_code v)
{
    switch(v)
    {
    case reply_code::succeeded:
        return "Succeeded";
    case reply_code::general_failure:
        return "General SOCKS server failure";
    case reply_code::connection_not_allowed_by_ruleset:
        return "Connection not allowed by ruleset";
    case reply_code::network_unreachable:
        return "Network unreachable";
    case reply_code::host_unreachable:
        return "Host unreachable";
    case reply_code::connection_refused:
        return "Connection refused";
    case reply_code::ttl_expired:
        return "TTL expired";
    case reply_code::command_not_supported:
        return "Command not supported";
    case reply_code::address_type_not_supported:
        return "Address type not supported";
    default:
        return "Unassigned";
    }

}

std::ostream&
operator<<(std::ostream& os, reply_code v)
{
    return os << to_string(v);
}

error_code
make_error_code(
    reply_code e) noexcept
{
    struct codes : error_category
    {
        const char*
        name() const noexcept override
        {
            return "boost.socks_proto.reply_code";
        }

        std::string
        message(int ev) const override
        {
            return to_string(
                to_reply_code(ev));
        }

        char const *
        message(
            int ev,
            char* buffer,
            std::size_t len ) const noexcept override
        {
            string_view msg = to_string(
                to_reply_code(ev));
            std::size_t n = msg.copy(buffer, len - 1);
            buffer[n] = '\0';
            return buffer;
        }

        error_condition
        default_error_condition(
            int ev) const noexcept override
        {
            return {
                static_cast<int>(to_reply_code(ev)),
                *this};
        }

        bool
        failed( int ev ) const noexcept override
        {
            return to_reply_code(ev) !=
                   reply_code::succeeded;
        }
    };

    static codes const cat{};
    return error_code{static_cast<
        std::underlying_type<reply_code>::type>(e), cat};
}

error_condition
make_error_condition(
    reply_code c) noexcept
{
    struct codes : error_category
    {
        const char*
        name() const noexcept override
        {
            return "boost.url.reply_code";
        }

        std::string
        message(int cv) const override
        {
            return to_string(to_reply_code(cv));
        }
    };
    static codes const cat{};
    return error_condition{static_cast<
        std::underlying_type<reply_code>::type>(c), cat};
}

} // http_proto
} // boost

#endif
