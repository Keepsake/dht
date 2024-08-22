# SPDX-License-Identifier: MIT

include(CMakePackageConfigHelpers)

set(package_dependencies [[
include(CMakeFindDependencyMacro)
find_dependency(asio CONFIG)
find_dependency(KsCrypto 1.1.2 CONFIG)
find_dependency(KsSerialization 1.1.2 CONFIG)
]])

write_basic_package_version_file(
  ${PROJECT_NAME}ConfigVersion.cmake
  COMPATIBILITY SameMajorVersion
)

export(
  TARGETS ks-dht
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
)

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/BuildTreeProjectConfig.cmake.in"
  ${PROJECT_NAME}Config.cmake
  @ONLY
)

export(PACKAGE ${PROJECT_NAME})

if(KS_DHT_INSTALL)
  install(
    TARGETS ks-dht
    FILE_SET headers
  )

  install(
    EXPORT ${PROJECT_NAME}Targets
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION share/${PROJECT_NAME}
  )

  configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/InstalledProjectConfig.cmake.in"
    installed_config/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION share/${PROJECT_NAME}
  )

  install(
    FILES
      "${PROJECT_BINARY_DIR}/installed_config/${PROJECT_NAME}Config.cmake"
      "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION share/${PROJECT_NAME}
  )
endif()
