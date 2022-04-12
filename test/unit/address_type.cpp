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
#include <boost/socks_proto/address_type.hpp>
#include <sstream>
#include "test_suite.hpp"

namespace boost {
namespace socks_proto {

class address_type_test
{
public:
    static
    void
    check(address_type v, string_view s)
    {
        std::stringstream ss;
        ss << v;
        BOOST_TEST(ss.str() == s);
    }

    void
    run()
    {
        check(address_type::ip_v4, "IPv4");
        check(address_type::domain_name, "Domain name");
        check(address_type::ip_v6, "IPv6");
    }
};

TEST_SUITE(
    address_type_test,
    "boost.socks_proto.address_type");

} // socks_proto
} // boost
