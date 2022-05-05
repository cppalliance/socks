#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
# Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/alandefreitas/socks_proto
#

set(EXTRAFILES
    ../../extra/test_main.cpp
    ../../extra/include/test_suite.hpp
    )

set(PFILES
    auth_options.cpp
    connect.cpp
    connect_v4.cpp
    endpoint.cpp
    error.cpp
    socks_proto.cpp
    string_view.cpp
    )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${PFILES})
source_group("_extra" FILES ${EXTRAFILES})
add_executable(boost_socks_proto_tests ${PFILES} ${EXTRAFILES})
target_include_directories(boost_socks_proto_tests PRIVATE . ../../extra/include)
target_link_libraries(boost_socks_proto_tests PRIVATE Boost::socks_proto)
add_test(NAME boost_socks_proto_tests COMMAND boost_socks_proto_tests)