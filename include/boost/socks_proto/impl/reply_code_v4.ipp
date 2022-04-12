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

} // http_proto
} // boost

#endif
