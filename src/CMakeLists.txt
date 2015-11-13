cmake_minimum_required(VERSION 3.3)
project("forward")

set(forward_source_root ${CMAKE_CURRENT_SOURCE_DIR})

include_directories("include/")
include_directories("core/")
include_directories("windows/")
include_directories("windows/dx11")
include_directories("math/")
include_directories("intersection/")
include_directories("geometry/")
include_directories("utilities/")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX")
    set(RenderAPILib "d3d11.lib D3DCompiler.lib dxgi.lib")
    set(SubSystem "/SUBSYSTEM:WINDOWS")
    add_definitions(-DUNICODE -D_UNICODE)
    MESSAGE( STATUS "MSVC")
endif()

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} ${RenderAPILib}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SubSystem}")


add_subdirectory(include)
add_subdirectory(core)
add_subdirectory(math)
add_subdirectory(geometry)
add_subdirectory(intersection)
add_subdirectory(utilities)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_subdirectory(windows)
endif()

