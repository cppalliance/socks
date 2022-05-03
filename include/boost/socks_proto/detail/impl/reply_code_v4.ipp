//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_REPLY_CODE_V4_IPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_REPLY_CODE_V4_IPP

#include <boost/socks_proto/detail/reply_code_v4.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace socks_proto {
namespace detail {

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

} // detail
} // http_proto
} // boost

#endif
