cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Winline -pedantic -std=c++17")

project(echo-server)

add_subdirectory(esutils)
add_subdirectory(server)
add_subdirectory(client)
