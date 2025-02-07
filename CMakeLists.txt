cmake_minimum_required(VERSION 3.30)
project(scrcpy-cpp-client)

set(CMAKE_CXX_STANDARD 26)

find_package(Boost 1.8.0 REQUIRED)

add_library(scrcpy-cpp-client STATIC src/client.hpp)
target_include_directories(scrcpy-cpp-client PUBLIC src)
set_target_properties(scrcpy-cpp-client PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(scrcpy-cpp-client PUBLIC ${Boost_LIBRARIES})

add_executable(scrcpy-cpp-client-test)
target_include_directories(scrcpy-cpp-client-test PUBLIC src)
target_sources(scrcpy-cpp-client-test PRIVATE src/test.cpp)
target_link_libraries(scrcpy-cpp-client)