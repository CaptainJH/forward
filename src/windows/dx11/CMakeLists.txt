cmake_minimum_required(VERSION 3.10)

find_source_files(forward_DX11_SOURCE_FILES "${forward_source_root}/windows")
remove_paths_from_list(forward_DX11_SOURCE_FILES "${forward_source_root}/windows/dx12")
remove_paths_from_list(forward_DX11_SOURCE_FILES "${forward_source_root}/windows/dx11_Hieroglyph")
set_source_groups("${forward_DX11_SOURCE_FILES}")

add_definitions(-DNOMINMAX -DDX11)
add_library(forwardDX11 STATIC ${forward_DX11_SOURCE_FILES})
set_property(TARGET forwardDX11 PROPERTY CXX_STANDARD 17)
target_link_libraries(forwardDX11 libForward)