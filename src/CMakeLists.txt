cmake_minimum_required(VERSION 3.10)

set(forward_source_root ${CMAKE_CURRENT_SOURCE_DIR})

#SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/Users/jhq/Downloads/boost_1_59_0")
#SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/Users/jhq/Downloads/boost_1_59_0/stage/lib")

#set(BOOST_ROOT "C:/")
#set(Boost_INCLUDE_DIR ${BOOST_ROOT})
#set(Boost_LIBRARY_DIR ${BOOST_ROOT})
#set(Boost_LIBRARY_DIR C:/Users/jhq/Downloads/boost_1_59_0/stage/lib)
find_package(Boost)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS}) 
	link_directories(${Boost_LIBRARY_DIRS})
	MESSAGE( STATUS "${Boost_INCLUDE_DIRS}")
	MESSAGE( STATUS "${Boost_LIBRARY_DIRS}")
	#target_link_libraries(progname ${Boost_LIBRARIES})
endif()

include_directories("${forward_repository_root}/libs/assimp/include/")
include_directories("${forward_source_root}/")
include_directories("include/")
include_directories("core/")
include_directories("windows/")
include_directories("math/")
include_directories("intersection/")
include_directories("geometry/")
include_directories("utilities/")
include_directories("render/")

option(CompileWithRenderDoc "RenderDoc integration")
set(RenderDocPath "" CACHE PATH "Specify the RenderDoc path")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX")
    set(RenderAPILibs "d3d11.lib d3d12.lib D3DCompiler.lib dxgi.lib dxguid.lib")
    set(OtherLibs "Dbghelp.lib")
    set(SubSystem "/SUBSYSTEM:WINDOWS")
    set(ParallelBuild "/MP")
    add_definitions(-DUNICODE -D_UNICODE)
    MESSAGE( STATUS "MSVC")

    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${ParallelBuild}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ParallelBuild}")

    link_directories("../libs/assimp/Win64")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
    set(SubSystem "/SUBSYSTEM:CONSOLE")

    MESSAGE( STATUS "Xcode")
endif()

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} ${RenderAPILibs} ${OtherLibs}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SubSystem}")


add_subdirectory(include)
add_subdirectory(core)
add_subdirectory(math)
add_subdirectory(geometry)
add_subdirectory(intersection)
add_subdirectory(utilities)
add_subdirectory(applications)
add_subdirectory(tools)


if(CompileWithRenderDoc)
    add_definitions(-DUSE_RENDERDOC)
	add_compile_definitions(RENDERDOC_PATH="${RenderDocPath}")
    include_directories("${RenderDocPath}")
endif()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_subdirectory(windows)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    add_subdirectory(osx)
endif()

find_source_files(forward_COMMON_SOURCE_FILES ${forward_source_root})
remove_paths_from_list(forward_COMMON_SOURCE_FILES "${forward_source_root}/applications")
remove_paths_from_list(forward_COMMON_SOURCE_FILES "${forward_source_root}/windows")
set_source_groups("${forward_COMMON_SOURCE_FILES}")

add_definitions(-DNOMINMAX)
add_library(libForward STATIC ${forward_COMMON_SOURCE_FILES})

