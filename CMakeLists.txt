cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0074 NEW)
project("forward")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(forward_repository_root ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${forward_repository_root}/cmake")
include(CMakeMacros)
include(CMakeFunctions)

set(Boost_ROOT "D:/dev/boost_1_83_0")
set(Boost_DIR ${Boost_ROOT})
set(Boost_INCLUDE_DIR ${Boost_ROOT})
set(Boost_LIBRARY_DIR ${Boost_ROOT}/stage/lib)
find_package(Boost REQUIRED)
find_package(SDL3 REQUIRED)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR}) 
	link_directories(${Boost_LIBRARY_DIR})
	MESSAGE( STATUS "${Boost_INCLUDE_DIR}")
	MESSAGE( STATUS "${Boost_LIBRARY_DIR}")
endif()

set(TBB_DIR "${forward_repository_root}/extern/src/oneTBB/lib/cmake/tbb/" CACHE PATH "Specify the oneTBB root path")
set(Tbb_INCLUDE_DIR "${forward_repository_root}/extern/src/oneTBB/include" CACHE PATH "")
set(Tbb_tbb_LIBRARY_DEBUG "${forward_repository_root}/extern/src/oneTBB/lib/intel64/vc14/tbb_debug.lib" CACHE PATH "")
set(Tbb_tbb_LIBRARY_RELEASE "${forward_repository_root}/extern/src/oneTBB/lib/intel64/vc14/tbb.lib" CACHE PATH "")
set(Tbb_tbbmalloc_LIBRARY_DEBUG "${forward_repository_root}/extern/src/oneTBB/lib/intel64/vc14/tbbmalloc_debug.lib" CACHE PATH "")
set(Tbb_tbbmalloc_LIBRARY_RELEASE "${forward_repository_root}/extern/src/oneTBB/lib/intel64/vc14/tbbmalloc.lib" CACHE PATH "")
set(Tbb_tbbmalloc_proxy_LIBRARY_DEBUG "${forward_repository_root}/extern/src/oneTBB/lib/intel64/vc14/tbbmalloc_proxy_debug.lib" CACHE PATH "")
set(Tbb_tbbmalloc_proxy_LIBRARY_RELEASE "${forward_repository_root}/extern/src/oneTBB/lib/intel64/vc14/tbbmalloc_proxy.lib" CACHE PATH "")
find_package(TBB 2021.10.0 REQUIRED)

add_subdirectory("src/")
add_subdirectory("extern/")