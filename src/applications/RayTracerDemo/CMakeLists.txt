cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(RayTracerDemo ${SOURCE_FILES})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set_target_properties(RayTracerDemo PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set_xcode_property(RayTracerDemo "CLANG_CXX_LANGUAGE_STANDARD" "c++17")
    set_xcode_property(RayTracerDemo "CLANG_CXX_LIBRARY" "libc++")
endif()
target_link_libraries(RayTracerDemo libForward)

set_property(TARGET RayTracerDemo PROPERTY CXX_STANDARD 17)
set_target_properties(RayTracerDemo PROPERTIES FOLDER Application)