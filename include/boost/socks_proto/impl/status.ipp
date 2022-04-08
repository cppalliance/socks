//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_STATUS_IPP
#define BOOST_HTTP_PROTO_IMPL_STATUS_IPP

#include <boost/socks_proto/status.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace socks_proto {

status
to_status(unsigned v)
{
    switch(static_cast<status>(v))
    {
    case status::request_granted:
    case status::general_failure:
    case status::connection_not_allowed_by_ruleset:
    case status::network_unreachable:
    case status::host_unreachable:
    case status::connection_refused_by_destination:
    case status::ttl_expired:
    case status::command_not_supported:
    case status::address_type_not_supported:
        return static_cast<status>(v);

    default:
        break;
    }
    return status::unknown;
}

string_view
to_string(status v)
{
    switch(static_cast<status>(v))
    {
    case status::request_granted:
        return "request_granted";
    case status::general_failure:
        return "general_failure";
    case status::connection_not_allowed_by_ruleset:
        return "connection_not_allowed_by_ruleset";
    case status::network_unreachable:
        return "network_unreachable";
    case status::host_unreachable:
        return "host_unreachable";
    case status::connection_refused_by_destination:
        return "connection_refused_by_destination";
    case status::ttl_expired:
        return "ttl_expired";
    case status::command_not_supported:
        return "command_not_supported";
    case status::address_type_not_supported:
        return "address_type_not_supported";
    default:
        break;
    }
    return "<unknown-status>";
}


std::ostream&
operator<<(std::ostream& os, status v)
{
    return os << to_string(v);
}

} // http_proto
} // boost

#endif
