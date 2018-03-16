cmake_minimum_required(VERSION 3.3)

find_source_files(forward_DX11_SOURCE_FILES "${forward_source_root}/windows")
remove_paths_from_list(forward_DX11_SOURCE_FILES "${forward_source_root}/windows/dx12")
remove_paths_from_list(forward_DX11_SOURCE_FILES "${forward_source_root}/windows/dx11")
set_source_groups("${forward_DX11_SOURCE_FILES}")

add_definitions(-DUSE_LEGACY_RENDERER)
add_library(forwardDX11_Hieroglyph STATIC ${forward_DX11_SOURCE_FILES})
target_link_libraries(forwardDX11_Hieroglyph libForward)