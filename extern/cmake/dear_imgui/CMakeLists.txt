cmake_minimum_required(VERSION 3.10)

project("dear_imgui")

include_directories("../../src/imgui")

find_source_files(dear_imgui_SOURCE_FILES "${forward_libs_root}/src/imgui")
remove_paths_from_list(dear_imgui_SOURCE_FILES "${forward_libs_root}/src/imgui/examples")
remove_paths_from_list(dear_imgui_SOURCE_FILES "${forward_libs_root}/src/imgui/backends")
remove_paths_from_list(dear_imgui_SOURCE_FILES "${forward_libs_root}/src/imgui/misc")
list(APPEND dear_imgui_SOURCE_FILES "${forward_libs_root}/src/imgui/backends/imgui_impl_sdl3.cpp")
list(APPEND dear_imgui_SOURCE_FILES "${forward_libs_root}/src/imgui/backends/imgui_impl_dx12.cpp")
list(APPEND dear_imgui_SOURCE_FILES "${forward_libs_root}/src/imgui/backends/imgui_impl_sdl3.h")
list(APPEND dear_imgui_SOURCE_FILES "${forward_libs_root}/src/imgui/backends/imgui_impl_dx12.h")
add_library(dear_imgui STATIC ${dear_imgui_SOURCE_FILES})
target_link_libraries(dear_imgui PRIVATE SDL3::SDL3)
set_property(TARGET dear_imgui PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET dear_imgui PROPERTY CXX_STANDARD 20)