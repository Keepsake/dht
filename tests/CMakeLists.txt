# SPDX-License-Identifier: MIT

add_executable(ks-dht-unit-tests
  common.cpp
  test_endpoint.cpp
  test_error.cpp
  test_id.cpp
  test_message.cpp
  test_peer.cpp
  test_routing_table.cpp
  test_socket.cpp
)

target_compile_definitions(ks-dht-unit-tests
  PRIVATE
    -DTESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(ks-dht-unit-tests
  PRIVATE
    asio::asio
    GTest::gmock
    GTest::gtest_main
    KsDht::ks-dht
    KsSerialization::ks-serialization
)

include(GoogleTest)
gtest_discover_tests(ks-dht-unit-tests
  DISCOVERY_MODE
    PRE_TEST
  XML_OUTPUT_DIR
    reports
)
