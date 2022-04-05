//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

// Test that header file is self-contained.
#include <boost/socks_proto/status.hpp>

#include "test_suite.hpp"

namespace boost {
namespace socks_proto {

class status_test
{
public:
    void
    testStatus()
    {
        auto const check = [&](status s, int i)
            {
                BOOST_TEST(int_to_status(i) == s);
            };
        check(status::request_granted, 0x00);
        check(status::general_failure, 0x01);
        check(status::connection_not_allowed_by_ruleset, 0x02);
        check(status::network_unreachable, 0x03);
        check(status::host_unreachable, 0x04);
        check(status::connection_refused_by_destination, 0x05);
        check(status::ttl_expired, 0x06);
        check(status::command_not_supported, 0x07);
        check(status::address_type_not_supported, 0x08);
        check(status::unknown, 0xFF);

        BOOST_TEST(int_to_status(0x09) == status::unknown);

        auto const good =
            [&](status v)
            {
                BOOST_TEST(to_string(v) != "<unknown-status>");
            };
        good(status::request_granted);
        good(status::general_failure);
        good(status::connection_not_allowed_by_ruleset);
        good(status::network_unreachable);
        good(status::host_unreachable);
        good(status::connection_refused_by_destination);
        good(status::ttl_expired);
        good(status::command_not_supported);
        good(status::address_type_not_supported);
    }

    void
    run()
    {
        testStatus();
    }
};

TEST_SUITE(status_test, "boost.socks_proto.status");

} // socks_proto
} // boost

