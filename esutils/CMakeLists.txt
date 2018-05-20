cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Winline -pedantic -std=c++17")

project(esutils)

file(GLOB UTILS_HEADER_FILES *.hpp)
file(GLOB UTILS_SOURCE_FILES *.cpp)

add_library(${PROJECT_NAME} SHARED ${UTILS_SOURCE_FILES} ${UTILS_HEADER_FILES})
install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})
