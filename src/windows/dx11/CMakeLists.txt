cmake_minimum_required(VERSION 3.3)

find_source_files(forward_DX11_SOURCE_FILES "${forward_source_root}/windows")
remove_paths_from_list(forward_DX11_SOURCE_FILES "${forward_source_root}/windows/dx12")
set_source_groups("${forward_DX11_SOURCE_FILES}")

add_library(forwardDX11 STATIC ${forward_DX11_SOURCE_FILES})
target_link_libraries(forwardDX11 libForward)