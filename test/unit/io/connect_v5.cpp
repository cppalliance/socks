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
#include <boost/socks_proto/io/connect_v5.hpp>
#include <sstream>
#include "test_suite.hpp"

namespace boost {
namespace socks_proto {

class io_connect_v5_test
{
public:
    void
    run()
    {
    }
};

TEST_SUITE(
    io_connect_v5_test,
    "boost.socks_proto.version");

} // socks_proto
} // boost
