//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_AUTH_HPP
#define BOOST_SOCKS_PROTO_AUTH_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/string_view.hpp>

namespace boost {
namespace socks_proto {
namespace auth {
    /// Options for no authentication operation
    struct no_auth
    {
    };

    /// Options for userpass authentication operation
    struct userpass
    {
        string_view user;
        string_view pass;
    };
} // auth
} // socks_proto
} // boost

#endif
