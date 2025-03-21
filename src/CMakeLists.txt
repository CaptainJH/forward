
set(forward_source_root ${CMAKE_CURRENT_SOURCE_DIR})

include_directories("${forward_source_root}/")
include_directories("${forward_repository_root}/extern/src/")
include_directories("${forward_repository_root}/extern/src/oneTBB/include/")
include_directories("${forward_repository_root}/extern/src/imgui")

option(forward_CompileWithRenderDoc "RenderDoc integration")
set(forward_RenderDocPath "" CACHE PATH "Specify the RenderDoc path")
option(forward_CompileWithWinPixEventRuntime "WinPixEventRuntime integration")
set(SuperluminalAPI_ROOT "" CACHE PATH "Specify the Superluminal root path")
option(forward_CompileWithSuperluminalAPI "SuperluminalAPI integration")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    include_directories("${forward_repository_root}/extern/src/assimp/include/")
    include_directories("${CMAKE_CURRENT_BINARY_DIR}/../extern/src/assimp/include/")
    include_directories("${forward_repository_root}/extern/src/DirectXTex/DirectXTex/")
  
    set(warnings "/W4 /WX")
    set(RenderAPILibs "d3d12.lib D3DCompiler.lib dxgi.lib dxguid.lib dxcompiler.lib")
    set(OtherLibs "Dbghelp.lib")
    set(SubSystem "/SUBSYSTEM:WINDOWS")
    set(ParallelBuild "/MP")
    add_definitions(-DUNICODE -D_UNICODE -DWINDOWS -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
    MESSAGE( STATUS "MSVC")
    
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${ParallelBuild}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ParallelBuild}")

    find_package(SuperluminalAPI)
    if(forward_CompileWithSuperluminalAPI AND SuperluminalAPI_FOUND)
        include_directories(${SuperluminalAPI_INCLUDE_DIRS})
        add_definitions(-DUSE_SUPERLUMINAL)
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    include_directories("macOS/")
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
    set(PLATFORM_OSX true)  
    add_definitions(-DMACOS)
    
    MESSAGE( STATUS "Xcode")
endif()
    
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} ${RenderAPILibs} ${OtherLibs}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SubSystem}")

if(forward_CompileWithWinPixEventRuntime)
    add_definitions(-DUSE_PIX)
endif()
if(forward_CompileWithRenderDoc)
    add_definitions(-DUSE_RENDERDOC)
	add_compile_definitions(RENDERDOC_PATH="${forward_RenderDocPath}")
    include_directories("${forward_RenderDocPath}")
endif()

add_subdirectory(math)
add_subdirectory(geometry)
add_subdirectory(utilities)
add_subdirectory(applications)
    
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_subdirectory(tools)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    add_subdirectory(macOS)
endif()

find_source_files(forward_SOURCE_FILES ${forward_source_root})
remove_paths_from_list(forward_SOURCE_FILES "${forward_source_root}/applications")
remove_paths_from_list(forward_SOURCE_FILES "${forward_source_root}/tools")
remove_paths_from_list(forward_SOURCE_FILES "${forward_source_root}/windows/dx11_Hieroglyph")
remove_paths_from_list(forward_SOURCE_FILES "${forward_source_root}/windows/dx11")
remove_paths_from_list(forward_SOURCE_FILES "${forward_source_root}/macOS")
remove_paths_from_list(forward_SOURCE_FILES "${forward_source_root}/MaterialXGenHlsl")
remove_paths_from_list(forward_SOURCE_FILES "${forward_source_root}/intersection")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    list(REMOVE_ITEM forward_SOURCE_FILES "${forward_source_root}/utilities/GeometryLoader.h")
    list(REMOVE_ITEM forward_SOURCE_FILES "${forward_source_root}/utilities/GeometryLoader.cpp")
endif()
set_source_groups("${forward_SOURCE_FILES}")

add_definitions(-DNOMINMAX -DDX12)
add_library(libForward STATIC ${forward_SOURCE_FILES})
set_property(TARGET libForward PROPERTY CXX_STANDARD 20)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set_xcode_property(libForward "CLANG_CXX_LANGUAGE_STANDARD" "c++17")
    set_xcode_property(libForward "CLANG_CXX_LIBRARY" "libc++")
endif()
target_link_libraries(libForward PUBLIC assimp DirectXTex OpenEXR dear_imgui)
target_include_directories(libForward INTERFACE $<INSTALL_INTERFACE:../src>)

install(
  TARGETS libForward
  EXPORT libForward-config
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
  EXPORT libForward-config
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  FILE libForward-targets.cmake
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  libForward-config.cmake.in libForward-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libForward-config.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})