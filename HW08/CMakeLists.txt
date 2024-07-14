cmake_minimum_required (VERSION 3.21) 
project (hw08 LANGUAGES C)

set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Debug")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

add_executable (hw08 listdir.c)
target_link_libraries (hw08 PRIVATE PkgConfig::GTK4)
