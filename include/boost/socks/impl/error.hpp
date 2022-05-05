//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_IMPL_ERROR_HPP
#define BOOST_SOCKS_IMPL_ERROR_HPP

namespace boost {
namespace system {

template<>
struct is_error_code_enum< ::boost::socks::error >
{
    static bool const value = true;
};
template<>
struct is_error_condition_enum< ::boost::socks::condition >
{
    static bool const value = true;
};

} // system
} // boost

namespace std {

template<>
struct is_error_code_enum< ::boost::socks::error >
{
    static bool const value = true;
};

template<>
struct is_error_condition_enum< ::boost::socks::condition >
{
    static bool const value = true;
};

} // std

namespace boost {
namespace socks {

BOOST_SOCKS_DECL
error_code
make_error_code(error e);

BOOST_SOCKS_DECL
error_condition
make_error_condition(condition c);

} // socks
} // boost

#endif
