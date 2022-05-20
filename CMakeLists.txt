#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
# Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/alandefreitas/socks_proto
#

cmake_minimum_required(VERSION 3.8...3.16)

set(BOOST_SOCKS_VERSION 1)
if(BOOST_SUPERPROJECT_VERSION)
    set(BOOST_SOCKS_VERSION ${BOOST_SUPERPROJECT_VERSION})
endif()

project(boost_socks VERSION "${BOOST_SOCKS_VERSION}" LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(CTest)
    option(BOOST_SOCKS_BUILD_TESTS "Build boost::socks tests" ${BUILD_TESTING})
    option(BOOST_SOCKS_BUILD_EXAMPLES "Build boost::socks examples" ON)
    set(BOOST_SOCKS_IS_ROOT ON)
else()
    set(BOOST_SOCKS_BUILD_TESTS ${BUILD_TESTING})
    set(BOOST_SOCKS_IS_ROOT OFF)
endif()

include(GNUInstallDirs)
if(BOOST_SOCKS_IS_ROOT)
    set(BOOST_INCLUDE_LIBRARIES socks asio beast url)
    set(BOOST_EXCLUDE_LIBRARIES socks)
    set(CMAKE_FOLDER _deps)
    add_subdirectory(../.. _deps/boost EXCLUDE_FROM_ALL)
    unset(CMAKE_FOLDER)
endif()

function(boost_socks_setup_properties target)
    target_compile_features(${target} PUBLIC cxx_constexpr)
    target_compile_definitions(${target} PUBLIC BOOST_SOCKS_NO_LIB=1)

    if(BOOST_SUPERPROJECT_VERSION)
        target_include_directories(${target} PUBLIC "${PROJECT_SOURCE_DIR}/include")
    else()
        target_include_directories(${target}
            PUBLIC
                "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        )
    endif()

    target_link_libraries(${target}
        PUBLIC
            Boost::asio
            Boost::core
            Boost::system
            Boost::exception
    )

    find_package (Threads)
    target_link_libraries(${target} PUBLIC Threads::Threads)

    if (MINGW)
        target_link_libraries(${target} PUBLIC ws2_32 mswsock)
    endif()

    if (WIN32 AND CMAKE_SYSTEM_VERSION)
        # Get windows version major and minor
        set(WIN_VERSION ${CMAKE_SYSTEM_VERSION})
        string(REGEX MATCH "^([0-9]+).([0-9])" WIN_VERSION ${WIN_VERSION})
        # Convert to HEX
        string(REGEX MATCH "^([0-9]+)" WIN_VERSION_MAJOR ${WIN_VERSION})
        if ("${WIN_VERSION_MAJOR}" MATCHES "10")
            set(WIN_VERSION_MAJOR "A")
            string(REGEX REPLACE "^([0-9]+)" ${WIN_VERSION_MAJOR} WIN_VERSION ${WIN_VERSION})
        elseif ("${WIN_VERSION_MAJOR}" MATCHES "11")
            set(WIN_VERSION_MAJOR "B")
            string(REGEX REPLACE "^([0-9]+)" ${WIN_VERSION_MAJOR} WIN_VERSION ${WIN_VERSION})
        endif()
        # Prepend single digits with 0
        string(REGEX REPLACE "^([0-9A-Z])\\." "0\\1." WIN_VERSION ${WIN_VERSION})
        string(REGEX REPLACE "([^0-9A-Z])([0-9A-Z])" "\\10\\2" WIN_VERSION ${WIN_VERSION})
        # Erase "."
        string(REPLACE "." "" WIN_VERSION ${WIN_VERSION})
        set(WIN32_WINNT_VALUE "0x${WIN_VERSION}")
        target_compile_definitions(${target} PUBLIC -D_WIN32_WINNT=${WIN32_WINNT_VALUE})
    endif()
endfunction()

file(GLOB_RECURSE BOOST_SOCKS_HEADERS CONFIGURE_DEPENDS
    include/boost/*.hpp
    include/boost/*.ipp
    include/boost/*.natvis
)

set(BOOST_SOCKS_SOURCES src/src.cpp)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/boost PREFIX "" FILES ${BOOST_SOCKS_HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "" FILES ${BOOST_SOCKS_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES Jamfile)

add_library(boost_socks ${BOOST_SOCKS_HEADERS} ${BOOST_SOCKS_SOURCES})
add_library(Boost::socks ALIAS boost_socks)
boost_socks_setup_properties(boost_socks)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(boost_socks PUBLIC BOOST_SOCKS_DYN_LINK=1)
else()
    target_compile_definitions(boost_socks PUBLIC BOOST_SOCKS_STATIC_LINK=1)
endif()

if(BOOST_SOCKS_BUILD_TESTS)
    add_subdirectory(test)
endif()

if(BOOST_SOCKS_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
