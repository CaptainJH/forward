cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(SmartPtrDemo ${SOURCE_FILES})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set_target_properties(SmartPtrDemo PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set_xcode_property(SmartPtrDemo "CLANG_CXX_LANGUAGE_STANDARD" "c++17")
    set_xcode_property(SmartPtrDemo "CLANG_CXX_LIBRARY" "libc++")
endif()
target_link_libraries(SmartPtrDemo libForward)

set_target_properties(SmartPtrDemo PROPERTIES FOLDER Application)