//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_ENDPOINT_HPP
#define BOOST_SOCKS_ENDPOINT_HPP

#include <boost/socks/detail/config.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace boost {
namespace socks {

/** Type used to represent endpoints

    Unlike Asio functions, which might use
    other `Endpoint` types, SOCKS can
    only connect to an `asio::ip::tcp::endpoint`
 */
using endpoint = asio::ip::tcp::endpoint;

} // socks
} // boost

#endif
