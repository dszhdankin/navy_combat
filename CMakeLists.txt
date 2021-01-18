cmake_minimum_required(VERSION 3.17)
project(navy_combat)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

add_subdirectory(src/graphical)
add_subdirectory(src/server)
add_subdirectory(src/sample_client)
add_subdirectory(src/tests)