cmake_minimum_required(VERSION 3.10)

project(Hello LANGUAGES C)

find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(my STATIC lib.c)

add_executable(hello main.c)

target_link_libraries(hello PUBLIC my ${CURL_LIBRARIES})
