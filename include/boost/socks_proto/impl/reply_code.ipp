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

} // http_proto
} // boost

#endif
