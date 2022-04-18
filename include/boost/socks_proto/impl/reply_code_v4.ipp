//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REPLY_CODE_V4_IPP
#define BOOST_HTTP_PROTO_IMPL_REPLY_CODE_V4_IPP

#include <boost/socks_proto/reply_code_v4.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace socks_proto {

reply_code_v4
to_reply_code_v4(unsigned v)
{
    switch(static_cast<reply_code_v4>(v))
    {
    case reply_code_v4::request_granted:
    case reply_code_v4::request_rejected_or_failed:
    case reply_code_v4::cannot_connect_to_identd_on_the_client:
    case reply_code_v4::client_and_identd_report_different_user_ids:
        return static_cast<reply_code_v4>(v);
    default:
        break;
    }
    return reply_code_v4::unassigned;
}

string_view
to_string(reply_code_v4 v)
{
    switch(v)
    {
    case reply_code_v4::request_granted:
        return "Request granted";
    case reply_code_v4::request_rejected_or_failed:
        return "General SOCKS server failure";
    case reply_code_v4::cannot_connect_to_identd_on_the_client:
        return "Connection not allowed by ruleset";
    case reply_code_v4::client_and_identd_report_different_user_ids:
        return "Network unreachable";
    default:
        return "Unassigned";
    }

}

std::ostream&
operator<<(std::ostream& os, reply_code_v4 v)
{
    return os << to_string(v);
}

error_code
make_error_code(
    reply_code_v4 e) noexcept
{
    struct codes : error_category
    {
        const char*
        name() const noexcept override
        {
            return "boost.socks_proto.reply_code_v4";
        }

        std::string
        message(int ev) const override
        {
            return to_string(
                to_reply_code_v4(ev));
        }

        char const *
        message(
            int ev,
            char* buffer,
            std::size_t len ) const noexcept override
        {
            string_view msg = to_string(
                to_reply_code(ev));
            msg.copy(buffer, len);
            return buffer;
        }

        error_condition
        default_error_condition(
            int ev) const noexcept override
        {
            return {
                static_cast<int>(to_reply_code_v4(ev)),
                *this};
        }

        bool
        failed( int ev ) const noexcept override
        {
            return to_reply_code_v4(ev) !=
                   reply_code_v4::request_granted;
        }
    };

    static codes const cat{};
    return error_code{static_cast<
        std::underlying_type<reply_code_v4>::type>(e), cat};
}

error_condition
make_error_condition(
    reply_code_v4 c) noexcept
{
    struct codes : error_category
    {
        const char*
        name() const noexcept override
        {
            return "boost.url.reply_code_v4";
        }

        std::string
        message(int cv) const override
        {
            return to_string(to_reply_code_v4(cv));
        }
    };
    static codes const cat{};
    return error_condition{static_cast<
        std::underlying_type<reply_code_v4>::type>(c), cat};
}


} // http_proto
} // boost

#endif
