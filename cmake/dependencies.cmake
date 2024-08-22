# SPDX-License-Identifier: MIT

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)

find_package(KsCMakeHelpers 3.0.0 CONFIG REQUIRED)

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(KsSerialization 1.1.2 CONFIG REQUIRED)
find_package(KsCrypto 1.1.2 CONFIG REQUIRED)

if(KS_DHT_BUILD_TEST)
  find_package(GTest 1.15.0 CONFIG REQUIRED)
endif()
