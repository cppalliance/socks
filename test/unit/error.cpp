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
#include <boost/socks/error.hpp>
#include <memory>
#include "test_suite.hpp"

namespace boost {
namespace socks {

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
                BOOST_TEST_NOT(ec.message().empty());
                BOOST_TEST(ec == c);
            }
            // condition
            {
                auto ec = make_error_condition(c);
                BOOST_TEST(ec.category().name() != nullptr);
                BOOST_TEST_NOT(ec.message().empty());
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
        check(condition::reply_error, error::unassigned_reply_code);
        check(condition::io_error, error::bad_reply_size);
        check(condition::io_error, error::bad_reply_version);
        check(condition::io_error, error::bad_server_choice);
        check(condition::io_error, error::bad_reply_command);
        check(condition::io_error, error::bad_reserved_component);
        check(condition::io_error, error::bad_address_type);
        check(condition::io_error, error::access_denied);
        check(condition::reply_error, error::unassigned_reply_code);

        error_code ec = static_cast<error>(0xEF);
        BOOST_TEST_EQ(
            ec.default_error_condition().value(), 0xEF);

        error_condition c = static_cast<condition>(0xEF);
        BOOST_TEST_NOT(c.message().empty());
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

TEST_SUITE(error_test, "boost.socks.error");

} // socks
} // boost
