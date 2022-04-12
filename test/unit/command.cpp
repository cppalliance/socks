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
#include <boost/socks_proto/command.hpp>
#include <sstream>
#include "test_suite.hpp"

namespace boost {
namespace socks_proto {

class command_test
{
public:
    static
    void
    check(command v, string_view s)
    {
        std::stringstream ss;
        ss << v;
        BOOST_TEST(ss.str() == s);
    }

    void
    run()
    {
        check(command::connect, "CONNECT");
        check(command::bind, "BIND");
        check(command::udp_associate, "UDP ASSOCIATE");
        check(command::unsupported, "UNSUPPORTED");
    }
};

TEST_SUITE(
    command_test,
    "boost.socks_proto.command");

} // socks_proto
} // boost
