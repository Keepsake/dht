include(GenerateExportHeader)
generate_export_header(ks-dht
  EXPORT_FILE_NAME ks/dht/detail/symbol_visibility.hpp
)

target_sources(
  ks-dht
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS
        ${PROJECT_BINARY_DIR}/include/
        ${PROJECT_SOURCE_DIR}/include/
      FILES
        ${PROJECT_BINARY_DIR}/include/ks/dht/detail/symbol_visibility.hpp
        ks/dht/detail/id.hpp
        ks/dht/detail/peer.hpp
        ks/dht/detail/routing_table.hpp
        ks/dht/endpoint.hpp
        ks/dht/error.hpp
        ks/dht/session.hpp
)

target_include_directories(
  ks-dht
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
