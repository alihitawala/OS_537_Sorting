cmake_minimum_required(VERSION 3.3)
project(Sorting)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SOURCE_FILES fastsort.c)
add_executable(Sorting ${SOURCE_FILES})