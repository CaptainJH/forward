cmake_minimum_required(VERSION 3.10)

set(forward_libs_root ${CMAKE_CURRENT_SOURCE_DIR})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")

add_subdirectory("src/assimp/")
set_property(TARGET assimp PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET assimp_cmd PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET zlibstatic PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET uninstall PROPERTY FOLDER "ThirdPartyLibraries")
#set_property(TARGET unit PROPERTY FOLDER "ThirdPartyLibraries")

add_subdirectory("cmake/dear_imgui/")
