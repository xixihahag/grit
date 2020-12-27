cmake_minimum_required(VERSION 2.8)

project(grit)
##################################################################################

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Werror -fPIC -g -pthread")

include_directories(${CMAKE_SOURCE_DIR}/third_part/include)
include_directories(${CMAKE_SOURCE_DIR}/transaction)
include_directories(${CMAKE_SOURCE_DIR}/base)
include_directories(${CMAKE_SOURCE_DIR}/flatbuffer)

link_directories(${CMAKE_SOURCE_DIR}/third_part/lib)

# add_subdirectory(flatbuffer)
add_subdirectory(transaction)