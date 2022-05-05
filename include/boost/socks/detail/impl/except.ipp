//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_DETAIL_IMPL_EXCEPT_IPP
#define BOOST_SOCKS_DETAIL_IMPL_EXCEPT_IPP

#include <boost/socks/detail/except.hpp>
#include <boost/version.hpp>
#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace boost {
namespace socks {
namespace detail {

void
throw_bad_alloc(
    source_location const& loc)
{
    throw_exception(
        std::bad_alloc(), loc);
}

void
throw_length_error(
    char const* what,
    source_location const& loc)
{
    throw_exception(
        std::length_error(what), loc);
}

void
throw_invalid_argument(
    char const* what,
    source_location const& loc)
{
    throw_exception(
        std::invalid_argument(what), loc);
}

void
throw_out_of_range(
    source_location const& loc)
{
    throw_exception(
        std::out_of_range("out of range"), loc);
}

void
throw_system_error(
    error_code const& ec,
    source_location const& loc)
{
    throw_exception(
        system_error(ec), loc);
}

} // detail
} // socks
} // boost

#endif
