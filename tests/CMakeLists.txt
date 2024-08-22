# SPDX-License-Identifier: MIT

add_library(ks-dht-impl INTERFACE)
add_library(KsDht::ks-dht-impl ALIAS ks-dht-impl)

target_sources(
  ks-dht-impl
  INTERFACE
    $<TARGET_OBJECTS:ks-dht>
)

target_link_libraries(
  ks-dht-impl
  INTERFACE
    $<TARGET_GENEX_EVAL:ks-dht,$<TARGET_PROPERTY:ks-dht,LINK_LIBRARIES>>
)

target_include_directories(
  ks-dht-impl
  INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
    $<TARGET_PROPERTY:ks-dht,INCLUDE_DIRECTORIES>
)

add_executable(ks-dht-impl-unit-tests
  common.cpp
  test_boost_to_std_error.cpp
  test_concurrent_guard.cpp
  test_discover_neighbors_task.cpp
  test_endpoint.cpp
  test_engine.cpp
  test_error.cpp
  test_fake_socket.cpp
  test_find_value_task.cpp
  test_first_session.cpp
  test_id.cpp
  test_ip_endpoint.cpp
  test_log.cpp
  test_lookup_task.cpp
  test_message.cpp
  test_message_serializer.cpp
  test_message_socket.cpp
  test_network.cpp
  test_notify_peer_task.cpp
  test_peer.cpp
  test_response_callbacks.cpp
  test_response_router.cpp
  test_routing_table.cpp
  test_session.cpp
  test_store_value_task.cpp
  test_timer.cpp
)

target_compile_definitions(ks-dht-impl-unit-tests
  PRIVATE
    -DTESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(ks-dht-impl-unit-tests
  PRIVATE
    asio::asio
    GTest::gmock
    GTest::gtest_main
    KsDht::ks-dht-impl
)

include(GoogleTest)
gtest_discover_tests(ks-dht-impl-unit-tests
  DISCOVERY_MODE
    PRE_TEST
  XML_OUTPUT_DIR
    reports
)
