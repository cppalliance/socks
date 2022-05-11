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
#include <boost/socks.hpp>
#include "test_suite.hpp"
#include "stream.hpp"

namespace boost {
namespace socks {

class snippets_test
{
public:
    void
    usingConnect()
    {
        asio::io_context ioc;
        test::stream socket(ioc);
        endpoint target_ep;

        {
            //[sync_connect
            error_code ec;
            endpoint bound_ep = connect(
                socket, target_ep, auth_options::none{}, ec);
            //]
            ignore_unused(bound_ep);
        }

        {
            auto do_write = [](const endpoint& bound_ep)
            {
                ignore_unused(bound_ep);
            };
            //[async_connect
            async_connect(
                socket, target_ep, auth_options::none{},
                [&](error_code ec, endpoint bound_ep)
            {
                // Continuation
                if (!ec.failed())
                    do_write(bound_ep);
            });
            //]
        }

        {
            //[sync_connect_v4
            error_code ec;
            endpoint bound_ep = connect_v4(
                socket, target_ep, "user_id", ec);
            //]
            ignore_unused(bound_ep);
        }

        {
            //[sync_connect_auth
            error_code ec;
            endpoint bound_ep = connect(
                socket,
                target_ep,
                auth_options::userpass{
                    "user_id", "password"},
                ec);
            //]
            ignore_unused(bound_ep);
        }

    }

    void
    run()
    {
        usingConnect();
    }
};

TEST_SUITE(snippets_test, "boost.socks.snippets");

} // socks
} // boost
