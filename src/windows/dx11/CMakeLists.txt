cmake_minimum_required(VERSION 3.3)

find_source_files(forward_DX11_SOURCE_FILES ${forward_source_root})
remove_paths_from_list(forward_DX11_SOURCE_FILES "${forward_source_root}/applications")
remove_paths_from_list(forward_DX11_SOURCE_FILES "${forward_source_root}/windows/dx12")
set_source_groups("${forward_DX11_SOURCE_FILES}")

add_library(forward STATIC ${forward_DX11_SOURCE_FILES})