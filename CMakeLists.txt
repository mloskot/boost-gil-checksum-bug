cmake_minimum_required(VERSION 3.10)
project(boost_gil_checksum_bug LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard version to use (default is 17)")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.70 REQUIRED)

add_executable(test_checksum_bug test_checksum_bug.cpp)
target_link_libraries(test_checksum_bug PRIVATE Boost::headers)
target_compile_definitions(test_checksum_bug PRIVATE BOOST_GIL_USE_CONCEPT_CHECK=1)
target_compile_options(test_checksum_bug PRIVATE $<$<CXX_COMPILER_ID:MSVC>:-FC>) # __FILE__
