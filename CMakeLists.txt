cmake_minimum_required(VERSION 3.3)

project(MockVendor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(mockvendor INTERFACE)

target_include_directories(mockvendor 
    SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(DIRECTORY include/MockVendor TYPE INCLUDE)
