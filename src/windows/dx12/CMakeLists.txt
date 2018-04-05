cmake_minimum_required(VERSION 3.3)

find_source_files(forward_DX12_SOURCE_FILES ${forward_source_root})
remove_paths_from_list(forward_DX12_SOURCE_FILES "${forward_source_root}/applications")
remove_paths_from_list(forward_DX12_SOURCE_FILES "${forward_source_root}/windows/dx11")
set_source_groups("${forward_DX12_SOURCE_FILES}")

add_library(forward_dx12 STATIC ${forward_DX12_SOURCE_FILES})