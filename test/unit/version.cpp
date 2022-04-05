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
#include <boost/socks_proto/version.hpp>
#include <sstream>
#include "test_suite.hpp"

namespace boost {
namespace socks_proto {

class version_test
{
public:
    void
    check(version v, string_view s)
    {
        std::stringstream ss;
        ss << v;
        BOOST_TEST(ss.str() == s);
    }

    void
    run()
    {
        check(version::socks_4_0, "SOCKS/4.0");
        check(version::socks_5_0, "SOCKS/5.0");
    }
};

TEST_SUITE(
    version_test,
    "boost.socks_proto.version");

} // socks_proto
} // boost
