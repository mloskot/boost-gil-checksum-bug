cmake_minimum_required(VERSION 3.10)
project(boost_gil_checksum_bug LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "C++ standard version to use (default is 11)")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.70 REQUIRED)

enable_testing()

add_executable(minimal_test minimal_test.cpp)
target_link_libraries(minimal_test PRIVATE Boost::headers)
target_compile_definitions(minimal_test PRIVATE BOOST_GIL_USE_CONCEPT_CHECK=1)
add_test(NAME minimal_test COMMAND minimal_test)

add_executable(complete_test complete_test.cpp)
target_link_libraries(complete_test PRIVATE Boost::headers)
target_compile_definitions(complete_test PRIVATE BOOST_GIL_USE_CONCEPT_CHECK=1)
add_test(NAME complete_test COMMAND complete_test "${CMAKE_CURRENT_BINARY_DIR}")
