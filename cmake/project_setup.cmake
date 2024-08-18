# SPDX-License-Identifier: MIT

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_CTEST_ARGUMENTS --output-on-failure --timeout 10)

add_definitions(-DPACKAGE_VERSION=${PROJECT_VERSION})
add_definitions(-DPACKAGE_BUGREPORT="david.keller@litchis.fr")

ks_setup_formatter(
  FILE_PATTERNS
    "include/*.[ch]pp"
    "src/*.[ch]pp"
    "tests/*.[ch]pp"
    "examples/*.[ch]pp"
)

enable_testing()
