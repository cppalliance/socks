//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

// Test that header file is self-contained.
#include <boost/socks_proto/auth_method.hpp>
#include <sstream>
#include "test_suite.hpp"

namespace boost {
namespace socks_proto {

class auth_method_test
{
public:
    static
    void
    check(auth_method v, string_view s)
    {
        std::stringstream ss;
        ss << v;
        BOOST_TEST(ss.str() == s);
    }

    void
    run()
    {
        check(auth_method::no_authentication, "No authentication");
        check(auth_method::gssapi, "Generic Security Services Application Program Interface");
        check(auth_method::userpass, "Username/password");
        check(auth_method::challenge_handshake, "Challenge-Handshake Authentication Protocol");
        check(auth_method::unassigned, "Unassigned");
        check(auth_method::challenge_response, "Challenge-Response Authentication Method");
        check(auth_method::ssl, "Secure Sockets Layer");
        check(auth_method::nds_authentication, "NDS Authentication");
        check(auth_method::multi_authentication_framework, "Multi-Authentication Framework");
        check(auth_method::json_parameter_block, "JSON Parameter Block");
        check(auth_method::private_authentication, "Methods reserved for private use");
        check(auth_method::no_acceptable_method, "No acceptable method");
    }
};

TEST_SUITE(
    auth_method_test,
    "boost.socks_proto.auth_method");

} // socks_proto
} // boost
