//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_STRING_VIEW_HPP
#define BOOST_SOCKS_PROTO_STRING_VIEW_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/socks_proto/byte.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost {
namespace socks_proto {

/// The type used to reference constant sequences of bytes
using byte_view = boost::core::basic_string_view<byte>;

} // socks_proto
} // boost

#endif
