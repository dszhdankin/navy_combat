cmake_minimum_required(VERSION 3.17)
project(navy_combat)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

add_subdirectory(src)