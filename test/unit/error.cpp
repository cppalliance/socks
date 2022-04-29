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
#include <boost/socks_proto/error.hpp>
#include <memory>
#include "test_suite.hpp"

namespace boost {
namespace socks_proto {

class error_test
{
public:
    void
    testErrors()
    {
        auto const check = [&](condition c, error e)
        {
            // code
            {
                auto const ec = make_error_code(e);
                BOOST_TEST(ec.category().name() != nullptr);
                BOOST_TEST(! ec.message().empty());
                BOOST_TEST(ec == c);
            }
            // condition
            {
                auto ec = make_error_condition(c);
                BOOST_TEST(ec.category().name() != nullptr);
                BOOST_TEST(! ec.message().empty());
                BOOST_TEST(ec == c);
            }

        };

        check(condition::succeeded, error::succeeded);
        check(condition::reply_error, error::general_failure);
        check(condition::reply_error, error::connection_not_allowed_by_ruleset);
        check(condition::reply_error, error::network_unreachable);
        check(condition::reply_error, error::host_unreachable);
        check(condition::reply_error, error::connection_refused);
        check(condition::reply_error, error::ttl_expired);
        check(condition::reply_error, error::command_not_supported);
        check(condition::reply_error, error::address_type_not_supported);
        check(condition::succeeded, error::request_granted);
        check(condition::reply_error, error::request_rejected_or_failed);
        check(condition::reply_error, error::cannot_connect_to_identd_on_the_client);
        check(condition::reply_error, error::client_and_identd_report_different_user_ids);
        check(condition::reply_error, error::unassigned);

    }

    void
    testStd()
    {
        std::error_code const ec = error::succeeded;
        BOOST_TEST(ec == error::succeeded);
        BOOST_TEST(ec == condition::succeeded);
    }

    void
    run()
    {
        testErrors();
        testStd();
    }
};

TEST_SUITE(error_test, "boost.socks_proto.error");

} // socks_proto
} // boost
