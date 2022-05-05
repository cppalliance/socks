//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#include <boost/socks.hpp>

#include "test_suite.hpp"

namespace boost {
namespace socks {

class limits_test
{
public:
    void
    testLimits()
    {
    }

    void
    run()
    {
        testLimits();
    }
};

TEST_SUITE(
    limits_test,
    "boost.socks.limits");

} // socks
} // boost
