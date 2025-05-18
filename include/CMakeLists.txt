include(GenerateExportHeader)
generate_export_header(ks-dht
  EXPORT_FILE_NAME ks/dht/detail/symbol_visibility.hpp
)

configure_file(ks/dht/detail/config.hpp.in ks/dht/detail/config.hpp @ONLY)

target_include_directories(
  ks-dht
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_sources(
  ks-dht
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ks/dht/detail/symbol_visibility.hpp"
        "${CMAKE_CURRENT_BINARY_DIR}/ks/dht/detail/config.hpp"
        ks/dht/detail/async_queue.hpp
        ks/dht/detail/id.hpp
        ks/dht/detail/peer.hpp
        ks/dht/detail/routing_table.hpp
        ks/dht/detail/socket.hpp
        ks/dht/endpoint.hpp
        ks/dht/error.hpp
        ks/dht/session.hpp
)
