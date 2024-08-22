# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.28)

project(KsDht VERSION 1.0.0 LANGUAGES CXX)

include(cmake/options.cmake)
include(cmake/dependencies.cmake)
include(cmake/project_setup.cmake)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(examples)
add_subdirectory(tests)

include(cmake/install.cmake)
