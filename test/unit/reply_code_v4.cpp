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
#include <boost/socks_proto/reply_code_v4.hpp>

#include "test_suite.hpp"

namespace boost {
namespace socks_proto {

class reply_code_v4_test
{
public:
    void
    testReplyCodeV4()
    {
        auto const check = [&](reply_code_v4 c, int i)
        {
            BOOST_TEST(to_reply_code_v4(i) == c);
        };
        check(reply_code_v4::request_granted, 90);
        check(reply_code_v4::request_rejected_or_failed, 91);
        check(reply_code_v4::cannot_connect_to_identd_on_the_client, 92);
        check(reply_code_v4::client_and_identd_report_different_user_ids, 93);
        check(reply_code_v4::unassigned, 0xFE);
        check(reply_code_v4::unassigned, 0xFF);
    }

    void
    run()
    {
        testReplyCodeV4();
    }
};

TEST_SUITE(reply_code_v4_test, "boost.socks_proto.reply_code_v4");

} // socks_proto
} // boost

