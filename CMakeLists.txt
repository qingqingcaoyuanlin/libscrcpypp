cmake_minimum_required(VERSION 3.25)
project(scrcpy++ VERSION 1.1.1)

cmake_policy(SET CMP0167 NEW)

set(CMAKE_CXX_STANDARD 23)

find_package(Boost 1.8.0 REQUIRED)

add_library(scrcpy++ STATIC src/client.hpp)
target_include_directories(scrcpy++ PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include/scrcpy++>
        PUBLIC ${BOOST_INCLUDE}
)
set_target_properties(scrcpy++ PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(scrcpy++ INTERFACE ${Boost_LIBRARIES})

add_executable(scrcpy++-test)
target_include_directories(scrcpy++-test PUBLIC src)
target_sources(scrcpy++-test PRIVATE src/test.cpp)
target_link_libraries(scrcpy++-test PRIVATE scrcpy++)

install(TARGETS scrcpy++
        ARCHIVE DESTINATION lib
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/scrcpy++Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/scrcpy++Config.cmake
        INSTALL_DESTINATION lib/cmake/scrcpy++
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/scrcpy++ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
install(TARGETS scrcpy++
        EXPORT scrcpy++Targets
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
)

install(EXPORT scrcpy++Targets
        FILE scrcpy++Targets.cmake
        NAMESPACE scrcpy::
        DESTINATION lib/cmake/scrcpy++
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/scrcpy++Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/scrcpy++ConfigVersion.cmake
        DESTINATION lib/cmake/scrcpy++
)

install(FILES src/client.hpp
        DESTINATION include/scrcpy++
)
