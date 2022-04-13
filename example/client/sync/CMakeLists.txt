#
# Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/alandefreitas/socks_proto
#

add_executable (socks-client-sync
        socks_client_sync.cpp
        )

target_link_libraries(socks-client-sync
        Boost::asio
        Boost::beast
        Boost::url
        Boost::socks_proto)

set_property(TARGET socks-client-sync PROPERTY FOLDER "example-socks-client-sync")