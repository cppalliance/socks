//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_IO_ENDPOINT_HPP
#define BOOST_SOCKS_PROTO_IO_ENDPOINT_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace boost {
namespace socks_proto {
namespace io {

/// Type used to represent string views
using endpoint = asio::ip::tcp::endpoint;

} // io
} // socks_proto
} // boost

#endif
