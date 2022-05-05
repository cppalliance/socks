//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_AUTH_OPTIONS_HPP
#define BOOST_SOCKS_AUTH_OPTIONS_HPP

#include <boost/socks/detail/config.hpp>
#include <boost/socks/string_view.hpp>

namespace boost {
namespace socks {

/** Authentication options for SOCKS5 requests

    No authentication (0x00) and
    username/password (0x02) are
    supported.

    @par References
    @li <a href="https://datatracker.ietf.org/doc/html/rfc1928#section-3">
        SOCKS Protocol Version 5: Procedure for TCP-based clients</a>
 */
struct auth_options {
    /// No authentication tag
    struct none {};

    /// Username/password tag and parameters
    struct userpass {
        string_view user;
        string_view pass;
    };

    /** Constructor
     */
    auth_options() = default;

    /** Constructor
     */
    auth_options( none const& ) : auth_options() {}

    /** Constructor
     */
    auth_options( userpass const& opt)
        : is_userpass(true)
        , user(opt.user)
        , pass(opt.pass)
    {}

    /** Return the authentication code
     */
    unsigned char
    code() const
    {
        return is_userpass ? 0x02 : 0x00;
    }

private:
    bool is_userpass{false};
    string_view user;
    string_view pass;
};

} // socks
} // boost

#endif
