cmake_minimum_required(VERSION 3.25)
project(scrcpy++)

set(CMAKE_CXX_STANDARD 23)

find_package(Boost 1.8.0 REQUIRED)

add_library(scrcpy++ STATIC src/client.hpp)
target_include_directories(scrcpy++ PUBLIC src)
set_target_properties(scrcpy++ PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(scrcpy++ INTERFACE ${Boost_LIBRARIES})

add_executable(scrcpy++-test)
target_include_directories(scrcpy++-test PUBLIC src)
target_sources(scrcpy++-test PRIVATE src/test.cpp)
target_link_libraries(scrcpy++)