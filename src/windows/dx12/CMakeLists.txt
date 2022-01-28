cmake_minimum_required(VERSION 3.10)

find_source_files(forward_DX12_SOURCE_FILES "${forward_source_root}/windows")
remove_paths_from_list(forward_DX12_SOURCE_FILES "${forward_source_root}/windows/dx11_Hieroglyph")
remove_paths_from_list(forward_DX12_SOURCE_FILES "${forward_source_root}/windows/dx11")
list(REMOVE_ITEM forward_DX12_SOURCE_FILES "${forward_source_root}/windows/dx12/ApplicationDX12.cpp")
list(REMOVE_ITEM forward_DX12_SOURCE_FILES "${forward_source_root}/windows/dx12/ApplicationDX12.h")
set_source_groups("${forward_DX12_SOURCE_FILES}")

add_definitions(-DNOMINMAX -DDX12)
add_library(forwardDX12 STATIC ${forward_DX12_SOURCE_FILES})
set_property(TARGET forwardDX12 PROPERTY CXX_STANDARD 17)
target_link_libraries(forwardDX12 libForward)