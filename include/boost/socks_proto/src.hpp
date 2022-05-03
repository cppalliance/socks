//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_SRC_HPP
#define BOOST_SOCKS_PROTO_SRC_HPP

/*
This file is meant to be included once,
in a translation unit of the program.
*/

#ifndef BOOST_SOCKS_PROTO_SOURCE
#define BOOST_SOCKS_PROTO_SOURCE
#endif

// We include this in case someone is
// using src.hpp as their main header file
#include <boost/socks_proto.hpp>

#include <boost/socks_proto/impl/connect.ipp>
#include <boost/socks_proto/impl/connect_v4.ipp>
#include <boost/socks_proto/impl/error.ipp>

#include <boost/socks_proto/detail/impl/address_type.ipp>
#include <boost/socks_proto/detail/impl/auth_method.ipp>
#include <boost/socks_proto/detail/impl/command.ipp>
#include <boost/socks_proto/detail/impl/version.ipp>
#include <boost/socks_proto/detail/impl/reply_code.ipp>
#include <boost/socks_proto/detail/impl/reply_code_v4.ipp>
#include <boost/socks_proto/detail/impl/except.ipp>

#endif

