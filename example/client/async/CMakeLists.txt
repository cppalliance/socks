#
# Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/alandefreitas/socks_proto
#

add_executable (socks-client-async
        socks_client_async.cpp
        )

target_link_libraries(socks-client-async
        Boost::asio
        Boost::beast
        Boost::url
        Boost::socks_proto)

target_include_directories(socks-client-async PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../sync)
set_property(TARGET socks-client-async PROPERTY FOLDER "example-socks-client-async")