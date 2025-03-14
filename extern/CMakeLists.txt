cmake_minimum_required(VERSION 3.10)

set(forward_libs_root ${CMAKE_CURRENT_SOURCE_DIR})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_INSTALL OFF CACHE BOOL "")
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "")
set(ASSIMP_NO_EXPORT OFF CACHE BOOL "")

add_subdirectory("src/assimp/")
set_property(TARGET assimp PROPERTY FOLDER "ThirdPartyLibraries")
# set_property(TARGET assimp_cmd PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET zlibstatic PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "ThirdPartyLibraries")
# set_property(TARGET uninstall PROPERTY FOLDER "ThirdPartyLibraries")
#set_property(TARGET unit PROPERTY FOLDER "ThirdPartyLibraries")

add_subdirectory("cmake/dear_imgui/")

set(BUILD_DX11 OFF CACHE BOOL "")
set(BUILD_SAMPLE OFF CACHE BOOL "")
set(BUILD_TESTING OFF CACHE BOOL "")
set(BUILD_TOOLS OFF CACHE BOOL "")
add_subdirectory("src/DirectXTex")
set_property(TARGET DirectXTex PROPERTY FOLDER "ThirdPartyLibraries")

add_subdirectory("src/MaterialX")
set_property(TARGET MaterialXCore PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET MaterialXFormat PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET MaterialXGenGlsl PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET MaterialXGenMdl PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET MaterialXGenOsl PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET MaterialXGenShader PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET MaterialXRender PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET MaterialXRenderGlsl PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET MaterialXRenderHw PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET MaterialXRenderOsl PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET MaterialXTest PROPERTY FOLDER "ThirdPartyLibraries")

add_subdirectory("src/Imath")
set_property(TARGET Imath PROPERTY FOLDER "ThirdPartyLibraries")

set(OPENEXR_BUILD_TOOLS OFF CACHE BOOL "")
set(OPENEXR_INSTALL_TOOLS OFF CACHE BOOL "")
set(OPENEXR_INSTALL_EXAMPLES OFF CACHE BOOL "")
add_subdirectory("src/OpenEXR")
set_property(TARGET OpenEXR PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET OpenEXRCore PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET OpenEXRUtil PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET IlmThread PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET Iex PROPERTY FOLDER "ThirdPartyLibraries")

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
set(BENCHMARK_USE_BUNDLED_GTEST OFF CACHE BOOL "")
add_subdirectory("src/google_benchmark")
set_property(TARGET benchmark PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET benchmark_main PROPERTY FOLDER "ThirdPartyLibraries")

# set(TEST_INCLUDE_BENCH_BITSHUFFLE_1 OFF CACHE BOOL "")
# set(TEST_INCLUDE_BENCH_BITSHUFFLE_N OFF CACHE BOOL "")
# set(TEST_INCLUDE_BENCH_SHUFFLE_1 OFF CACHE BOOL "")
# set(TEST_INCLUDE_BENCH_SHUFFLE_N OFF CACHE BOOL "")
# set(TEST_INCLUDE_COMPAT OFF CACHE BOOL "")
# set(BUILD_TESTS OFF CACHE BOOL "")
# set(BUILD_BENCHMARKS OFF CACHE BOOL "")
# set(BUILD_SHARED OFF CACHE BOOL "") 
# set(BUILD_FUZZERS OFF CACHE BOOL "")
# add_subdirectory("src/c-blosc")
# set_property(TARGET blosc_static PROPERTY FOLDER "ThirdPartyLibraries")

# set(Blosc_INCLUDE_DIR "${forward_repository_root}/extern/src/c-blosc/blosc" CACHE PATH "")
# set(Blosc_LIBRARY_DEBUG "${forward_repository_root}/build/extern/src/c-blosc/blosc/Debug/libbloscd.lib" CACHE PATH "")
# set(Blosc_LIBRARY_RELEASE "${forward_repository_root}/build/extern/src/c-blosc/blosc/Release/libblosc.lib" CACHE PATH "")
# set(Boost_IOSTREAMS_LIBRARY_DEBUG "${Boost_ROOT}/stage/lib/libboost_iostreams-vc143-mt-gd-x64-1_83.lib" CACHE PATH "")
# set(Boost_IOSTREAMS_LIBRARY_RELEASE "${Boost_ROOT}/stage/lib/libboost_iostreams-vc143-mt-x64-1_83.lib" CACHE PATH "")
# set(Boost_REGEX_LIBRARY_DEBUG "${Boost_ROOT}/stage/lib/libboost_regex-vc143-mt-gd-x64-1_83.lib" CACHE PATH "")
# set(Boost_REGEX_LIBRARY_RELEASE "${Boost_ROOT}/stage/lib/libboost_regex-vc143-mt-x64-1_83.lib" CACHE PATH "")
# set(OPENVDB_CORE_STATIC OFF CACHE BOOL "")
# set(OPENVDB_ENABLE_UNINSTALL OFF CACHE BOOL "")
# set(ZLIB_INCLUDE_DIR "${forward_repository_root}/extern/src/assimp/contrib/zlib" CACHE PATH "")
# set(ZLIB_LIBRARY_DEBUG "${forward_repository_root}/build/extern/src/assimp/contrib/zlib/Debug/zlibstaticd.lib" CACHE PATH "")
# set(ZLIB_LIBRARY_RELEASE "${forward_repository_root}/build/extern/src/assimp/contrib/zlib/Release/zlibstatic.lib" CACHE PATH "")
# include_directories("${forward_repository_root}/build/extern/src/assimp/contrib/zlib")
# add_subdirectory("src/OpenVDB")
# set_property(TARGET openvdb_shared PROPERTY FOLDER "ThirdPartyLibraries")
# set_property(TARGET vdb_print PROPERTY FOLDER "ThirdPartyLibraries")