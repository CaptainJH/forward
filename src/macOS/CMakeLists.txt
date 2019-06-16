cmake_minimum_required(VERSION 3.10)

find_source_files(forward_Metal_SOURCE_FILES "${forward_source_root}/macOS")
list(REMOVE_ITEM forward_Metal_SOURCE_FILES "${forward_source_root}/macOS/MetalViewController.h")
list(REMOVE_ITEM forward_Metal_SOURCE_FILES "${forward_source_root}/macOS/MetalViewController.mm")
set_source_groups("${forward_Metal_SOURCE_FILES}")

add_definitions(-DNOMINMAX)
add_library(forwardMetal STATIC ${forward_Metal_SOURCE_FILES})
target_link_libraries(forwardMetal libForward)
target_link_libraries(forwardMetal "-framework MetalPerformanceShaders")
target_link_libraries(forwardMetal "-framework Metal")
target_link_libraries(forwardMetal "-framework MetalKit")
target_link_libraries(forwardMetal "-framework ModelIO")

set_xcode_property(forwardMetal "CLANG_CXX_LANGUAGE_STANDARD" "c++17")
set_xcode_property(forwardMetal "CLANG_CXX_LIBRARY" "libc++")