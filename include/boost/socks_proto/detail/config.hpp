//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_DETAIL_CONFIG_HPP
#define BOOST_SOCKS_PROTO_DETAIL_CONFIG_HPP

#include <boost/config.hpp>
#include <stdint.h>

namespace boost {

namespace socks_proto {

#if defined(BOOST_SOCKS_PROTO_DOCS)
# define BOOST_SOCKS_PROTO_DECL
# define BOOST_SOCKS_PROTO_PROTECTED private
#else
# define BOOST_SOCKS_PROTO_PROTECTED protected
# if (defined(BOOST_SOCKS_PROTO_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)) && !defined(BOOST_SOCKS_PROTO_STATIC_LINK)
#  if defined(BOOST_SOCKS_PROTO_SOURCE)
#   define BOOST_SOCKS_PROTO_DECL        BOOST_SYMBOL_EXPORT
#   define BOOST_SOCKS_PROTO_BUILD_DLL
#  else
#   define BOOST_SOCKS_PROTO_DECL        BOOST_SYMBOL_IMPORT
#  endif
# endif // shared lib
# ifndef  BOOST_SOCKS_PROTO_DECL
#  define BOOST_SOCKS_PROTO_DECL
# endif
# if !defined(BOOST_SOCKS_PROTO_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_SOCKS_PROTO_NO_LIB)
#  define BOOST_LIB_NAME boost_socks_proto
#  if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_SOCKS_PROTO_DYN_LINK)
#   define BOOST_DYN_LINK
#  endif
#  include <boost/config/auto_link.hpp>
# endif
#endif

#if 0 // defined but not used yet
using off_t = ::uint16_t; // private

// maximum size of http header,
// chunk header, or chunk extensions
#ifndef BOOST_SOCKS_PROTO_MAX_HEADER
#define BOOST_SOCKS_PROTO_MAX_HEADER 65535U
#endif
static constexpr auto max_off_t =
    BOOST_SOCKS_PROTO_MAX_HEADER;
#endif

} // socks_proto

} // boost

#endif
