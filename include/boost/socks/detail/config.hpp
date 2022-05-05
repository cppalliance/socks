//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_DETAIL_CONFIG_HPP
#define BOOST_SOCKS_DETAIL_CONFIG_HPP

#include <boost/config.hpp>
#include <stdint.h>

namespace boost {

namespace socks {

#if defined(BOOST_SOCKS_DOCS)
# define BOOST_SOCKS_DECL
# define BOOST_SOCKS_PROTECTED private
#else
# define BOOST_SOCKS_PROTECTED protected
# if (defined(BOOST_SOCKS_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)) && !defined(BOOST_SOCKS_STATIC_LINK)
#  if defined(BOOST_SOCKS_SOURCE)
#   define BOOST_SOCKS_DECL        BOOST_SYMBOL_EXPORT
#   define BOOST_SOCKS_BUILD_DLL
#  else
#   define BOOST_SOCKS_DECL        BOOST_SYMBOL_IMPORT
#  endif
# endif // shared lib
# ifndef  BOOST_SOCKS_DECL
#  define BOOST_SOCKS_DECL
# endif
# if !defined(BOOST_SOCKS_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_SOCKS_NO_LIB)
#  define BOOST_LIB_NAME boost_socks
#  if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_SOCKS_DYN_LINK)
#   define BOOST_DYN_LINK
#  endif
#  include <boost/config/auto_link.hpp>
# endif
#endif

/*
    Asio async result types, such as:

    typename asio::async_result<
        typename asio::decay<CompletionToken>::type,
        void (error_code, endpoint)
    >::return_type
 */
#ifndef BOOST_SOCKS_ASYNC_ENDPOINT
# define BOOST_SOCKS_ASYNC_ENDPOINT(type) \
     BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(type, void(::boost::socks::error_code, ::boost::asio::ip::tcp::endpoint))
#endif

} // socks

} // boost

#endif
