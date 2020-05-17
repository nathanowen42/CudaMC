cmake_minimum_required(VERSION 2.8.9)
project (CudaMC)

set(CMAKE_CXX_COMPILER "/usr/bin/clang++-10")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -pedantic")

include_directories(../include)

file(GLOB SOURCES "../src/*.cpp")

#add_library(cudamc SHARED ${SOURCES})
add_executable(cudamc ${SOURCES})
