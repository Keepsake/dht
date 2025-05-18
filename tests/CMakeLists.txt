# SPDX-License-Identifier: MIT

include(CheckSourceCompiles)

configure_file(test_config.hpp.in test_config.hpp @ONLY)

add_executable(ks-dht-unit-tests
  async_queue.test.cpp
  bucket.test.cpp
  candidate_tracker.test.cpp
  discover_neighbors_task.test.cpp
  endpoint.test.cpp
  error.test.cpp
  find_value_task.test.cpp
  id.test.cpp
  message.test.cpp
  network.test.cpp
  notify_peer_task.test.cpp
  peer.test.cpp
  request_tracker.test.cpp
  routing_table.test.cpp
  scope_exit.test.cpp
  socket.test.cpp
)

target_include_directories(ks-dht-unit-tests
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
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
