# Use, modification, and distribution are
# subject to the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt)
#
# Copyright Rene Rivera 2020.
# Copyright Alan de Freitas 2022.

# For Drone CI we use the Starlark scripting language to reduce duplication.
# As the yaml syntax for Drone CI is rather limited.
#
#
globalenv={'B2_CI_VERSION': '1', 'B2_VARIANT': 'release'}
linuxglobalimage="cppalliance/droneubuntu1804:1"
windowsglobalimage="cppalliance/dronevs2019"

def main(ctx):
  return [
    # Coverage
    linux_cxx("codecov", "g++-8", packages="g++-8", buildtype="codecov", buildscript="drone", image=linuxglobalimage, environment={'COMMENT': 'codecov.io', 'B2_CXXSTD': '11', 'B2_TOOLSET': 'gcc-8', 'B2_DEFINES': 'BOOST_NO_STRESS_TEST=1', 'CODECOV_TOKEN': {'from_secret': 'codecov_token'}, 'DRONE_JOB_UUID': 'b6589fc6ab'}, globalenv=globalenv),

    # Docs
    linux_cxx("docs", "g++", packages="docbook docbook-xml docbook-xsl xsltproc libsaxonhe-java default-jre-headless flex libfl-dev bison unzip rsync", buildtype="docs", buildscript="drone", image="cppalliance/droneubuntu1804:1", environment={'COMMENT': 'docs', 'DRONE_JOB_UUID': 'b6589fc6ab'}, globalenv=globalenv),

  # Sanitizers + Valgrind
  linux_cxx("ASan", "g++-12", packages="g++-12", buildscript="drone", buildtype="boost", image="cppalliance/droneubuntu2204:1", environment={'COMMENT': 'asan', 'B2_VARIANT': 'debug', 'B2_TOOLSET': 'gcc-12', 'B2_CXXSTD': '11,14,17', 'B2_ASAN': '1', 'B2_DEFINES': 'BOOST_NO_STRESS_TEST=1', 'DRONE_EXTRA_PRIVILEGED': 'True'}, globalenv=globalenv, privileged=True),
  linux_cxx("TSan", "g++-12", packages="g++-12", buildscript="drone", buildtype="boost", image="cppalliance/droneubuntu2204:1", environment={'COMMENT': 'tsan', 'B2_VARIANT': 'debug', 'B2_TOOLSET': 'gcc-12', 'B2_CXXSTD': '11,14,17', 'B2_TSAN': '1', 'B2_DEFINES': 'BOOST_NO_STRESS_TEST=1'}, globalenv=globalenv),
  linux_cxx("UBSan", "g++-12", packages="g++-12", buildscript="drone", buildtype="boost", image="cppalliance/droneubuntu2204:1", environment={'COMMENT': 'ubsan', 'B2_VARIANT': 'debug', 'B2_TOOLSET': 'gcc-12', 'B2_CXXSTD': '11,14,17', 'B2_UBSAN': '1', 'B2_DEFINES': 'define=BOOST_NO_STRESS_TEST=1 define=BOOST_URL_HEADER_ONLY=1', 'B2_LINKFLAGS': '-fuse-ld=gold'}, globalenv=globalenv),
  linux_cxx("Valgrind", "clang++-14", packages="clang-14 libc6-dbg libc++-dev libstdc++-9-dev", llvm_os="jammy", llvm_ver="14", buildscript="drone", buildtype="valgrind", image="cppalliance/droneubuntu2204:1", environment={'COMMENT': 'valgrind', 'B2_TOOLSET': 'clang-14', 'B2_CXXSTD': '11,14,17', 'B2_DEFINES': 'BOOST_NO_STRESS_TEST=1', 'B2_VARIANT': 'debug', 'B2_TESTFLAGS': 'testing.launcher=valgrind', 'VALGRIND_OPTS': '--error-exitcode=1'}, globalenv=globalenv),

    # GCC
    linux_cxx("gcc 4.8", "g++-4.8", packages="g++-4.8", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'B2_TOOLSET': 'gcc-4.8', 'B2_CXXSTD': '11', 'DRONE_JOB_UUID': 'ac3478d69a'}, globalenv=globalenv),
    linux_cxx("gcc 4.9", "g++-4.9", packages="g++-4.9", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'B2_TOOLSET': 'gcc-4.9', 'B2_CXXSTD': '11', 'DRONE_JOB_UUID': 'c1dfd96eea'}, globalenv=globalenv),
    linux_cxx("gcc 5", "g++-5", packages="g++-5", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'gcc-5', 'B2_CXXSTD': '11', 'DRONE_JOB_UUID': '902ba3cda1'}, globalenv=globalenv),
    linux_cxx("gcc 6", "g++-6", packages="g++-6", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'gcc-6', 'B2_CXXSTD': '11,14', 'DRONE_JOB_UUID': 'fe5dbbcea5'}, globalenv=globalenv),
    linux_cxx("gcc 7", "g++-7", packages="g++-7", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'gcc-7', 'B2_CXXSTD': '14,17', 'DRONE_JOB_UUID': '0ade7c2cf9'}, globalenv=globalenv),
    linux_cxx("gcc 8", "g++-8", packages="g++-8", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'gcc-8', 'B2_CXXSTD': '17,2a', 'DRONE_JOB_UUID': 'b1d5781111'}, globalenv=globalenv),
    linux_cxx("gcc 9", "g++-9", packages="g++-9", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'gcc-9', 'B2_CXXSTD': '17,2a', 'DRONE_JOB_UUID': '17ba079149'}, globalenv=globalenv),
    linux_cxx("gcc 10", "g++-10", packages="g++-10", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2004:1", environment={'B2_TOOLSET': 'gcc-10', 'B2_CXXSTD': '17,2a', 'DRONE_JOB_UUID': '17ba079159'}, globalenv=globalenv),
    linux_cxx("gcc 11", "g++-11", packages="g++-11", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2004:1", environment={'B2_TOOLSET': 'gcc-11', 'B2_CXXSTD': '17,2a', 'DRONE_JOB_UUID': '17ba079169'}, globalenv=globalenv),
    linux_cxx("gcc 12", "g++-12", packages="g++-12", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2204:1", environment={'B2_TOOLSET': 'gcc-12', 'B2_CXXSTD': '17,20'}, globalenv=globalenv),

    # Clang
    linux_cxx("clang 3.8", "clang++-3.8", packages="clang-3.8", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'B2_TOOLSET': 'clang-3.8', 'B2_CXXSTD': '11', 'DRONE_JOB_UUID': '7b52009b64'}, globalenv=globalenv),
    linux_cxx("clang 4.0", "clang++-4.0", packages="clang-4.0 libstdc++-6-dev", llvm_os="xenial", llvm_ver="4.0", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'B2_TOOLSET': 'clang-4.0', 'B2_CXXSTD': '11,14', 'DRONE_JOB_UUID': 'bd307a3ec3'}, globalenv=globalenv),
    linux_cxx("clang 5.0", "clang++-5.0", packages="clang-5.0 libstdc++-7-dev", llvm_os="bionic", llvm_ver="5.0", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'clang-5.0', 'B2_CXXSTD': '11,14', 'DRONE_JOB_UUID': 'fa35e19212'}, globalenv=globalenv),
    linux_cxx("clang 6.0", "clang++-6.0", packages="clang-6.0 libc6-dbg libc++-dev libstdc++-8-dev", llvm_os="bionic", llvm_ver="6.0", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'clang-6.0', 'B2_CXXSTD': '14,17', 'DRONE_JOB_UUID': 'f1abd67035'}, globalenv=globalenv),
    linux_cxx("clang 7", "clang++-7", packages="clang-7 libc6-dbg libc++-dev libstdc++-8-dev", llvm_os="bionic", llvm_ver="7", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'clang-7', 'B2_CXXSTD': '17,2a', 'DRONE_JOB_UUID': '1574bddb75'}, globalenv=globalenv),
    linux_cxx("clang 8", "clang++-8", packages="clang-8 libc6-dbg libc++-dev libstdc++-8-dev", llvm_os="bionic", llvm_ver="8", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'clang-8', 'B2_CXXSTD': '17,2a', 'DRONE_JOB_UUID': '0716d9708d'}, globalenv=globalenv),
    linux_cxx("clang 9", "clang++-9", packages="clang-9 libc6-dbg libc++-dev libstdc++-9-dev", llvm_os="bionic", llvm_ver="9", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'clang-9', 'B2_CXXSTD': '11,14,17,2a', 'DRONE_JOB_UUID': '9e6a55b6b4'}, globalenv=globalenv),
    linux_cxx("clang 10", "clang++-10", packages="clang-10 libc6-dbg libc++-dev libstdc++-9-dev", llvm_os="focal", llvm_ver="10", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2004:1", environment={'B2_TOOLSET': 'clang-10', 'B2_CXXSTD': '11,14,17,2a', 'DRONE_JOB_UUID': '9e6a55b6c4'}, globalenv=globalenv),
    linux_cxx("clang 11", "clang++-11", packages="clang-11 libc6-dbg libc++-dev libstdc++-9-dev", llvm_os="focal", llvm_ver="11", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2004:1", environment={'B2_TOOLSET': 'clang-11', 'B2_CXXSTD': '11,14,17,2a', 'DRONE_JOB_UUID': '9e6a55b6b4'}, globalenv=globalenv),
    linux_cxx("clang 12", "clang++-12", packages="clang-12 libc6-dbg libc++-dev libstdc++-9-dev", llvm_os="focal", llvm_ver="12", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2004:1", environment={'B2_TOOLSET': 'clang-12', 'B2_CXXSTD': '11,14,17,20', 'DRONE_JOB_UUID': '9e6a55b6b5'}, globalenv=globalenv),
    linux_cxx("clang 13", "clang++-13", packages="clang-13 libc6-dbg libc++-dev libstdc++-10-dev", llvm_os="jammy", llvm_ver="13", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2204:1", environment={'B2_TOOLSET': 'clang-13', 'B2_CXXSTD': '11,14,17,20'}, globalenv=globalenv),
    linux_cxx("clang 14", "clang++-14", packages="clang-14 libc6-dbg libc++-dev libstdc++-10-dev", llvm_os="jammy", llvm_ver="14", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2204:1", environment={'B2_TOOLSET': 'clang-14', 'B2_CXXSTD': '11,14,17,20'}, globalenv=globalenv),
    linux_cxx("clang 15", "clang++-15", packages="clang-15 libc6-dbg libc++-dev libstdc++-10-dev", llvm_os="jammy", llvm_ver="15", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2204:1", environment={'B2_TOOLSET': 'clang-15', 'B2_CXXSTD': '11,14,17,20'}, globalenv=globalenv),
    linux_cxx("clang 6.0 libc++", "clang++-6.0", packages="clang-6.0 libc6-dbg libc++-dev libc++abi-dev libstdc++-8-dev", llvm_os="bionic", llvm_ver="6.0", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_TOOLSET': 'clang-6.0', 'B2_CXXSTD': '11,14', 'B2_STDLIB': 'libc++', 'DRONE_JOB_UUID': 'b3f0c7f6bb'}, globalenv=globalenv),
    osx_cxx("clang", "g++", packages="", buildtype="boost", buildscript="drone", environment={'B2_TOOLSET': 'clang', 'B2_CXXSTD': '11,17', 'DRONE_JOB_UUID': '91032ad7bb'}, globalenv=globalenv),
    linux_cxx("coverity", "g++", packages="", buildtype="coverity", buildscript="drone", image=linuxglobalimage, environment={'COMMENT': 'Coverity Scan', 'B2_TOOLSET': 'clang', 'DRONE_JOB_UUID': '472b07b9fc'}, globalenv=globalenv),

    # MSVC
    windows_cxx("msvc-14.1", "", image="cppalliance/dronevs2017", buildtype="boost", buildscript="drone", environment={"B2_TOOLSET": "msvc-14.1", "B2_CXXSTD": "11,14,17"},globalenv=globalenv),
    windows_cxx("msvc-14.2", "", image="cppalliance/dronevs2019", buildtype="boost", buildscript="drone", environment={"B2_TOOLSET": "msvc-14.2", "B2_CXXSTD": "14,17,latest"},globalenv=globalenv),

    # arm64
    linux_cxx("gcc-11 arm64", "g++-11", packages="g++-11", buildscript="drone", buildtype="boost", image="cppalliance/droneubuntu2004:multiarch", environment={'B2_TOOLSET': 'gcc-11', 'B2_CXXSTD': '17,20', 'DRONE_JOB_UUID': 'fa35e19219'}, arch="arm64", globalenv=globalenv),
    linux_cxx("clang-12 arm64", "clang++-12", packages="clang-12 libstdc++-9-dev", llvm_os="focal", llvm_ver="12", buildscript="drone", buildtype="boost", image="cppalliance/droneubuntu2004:multiarch", environment={'B2_TOOLSET': 'clang-12', 'B2_CXXSTD': '11,14,17,20', 'DRONE_JOB_UUID': '472b07ba03'}, arch="arm64", globalenv=globalenv),

    # s390x
    linux_cxx("Clang 12 s390x", "clang++-12", packages="clang-12 libstdc++-9-dev", llvm_os="focal", llvm_ver="12", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2004:multiarch", environment={'B2_TOOLSET': 'clang-12', 'B2_CXXSTD': '17,20'}, arch="s390x", globalenv=globalenv),
    linux_cxx("gcc 11 s390x", "g++-11", packages="g++-11", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu2004:multiarch", environment={'B2_TOOLSET': 'gcc-11', 'B2_CXXSTD': '17,2a'}, arch="s390x", globalenv=globalenv),
  ]

# from https://github.com/boostorg/boost-ci
load("@boost_ci//ci/drone/:functions.star", "linux_cxx","windows_cxx","osx_cxx","freebsd_cxx")
