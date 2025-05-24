cmake_minimum_required(VERSION 3.9)

project(MockVendor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(mockvendor INTERFACE)

target_include_directories(mockvendor 
    SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(DIRECTORY include/MockVendor TYPE INCLUDE)

if (${MOCKVENDOR_TESTS})
    enable_testing()

    include(GoogleTest)
    find_package(GTest)

    add_executable(
      test_mockvendor
      tests/mockvendor_tests.cpp
    )
    target_link_libraries(
      test_mockvendor
      GTest::gmock_main
    )
    target_include_directories(test_mockvendor
        PRIVATE ${CMAKE_SOURCE_DIR}/include
    )

    gtest_discover_tests(test_mockvendor)
endif()
