cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_executable(BasicGeometryFrameGraph ${SOURCE_FILES})
    target_link_libraries(BasicGeometryFrameGraph forwardDX11)
    target_link_libraries(BasicGeometryFrameGraph forwardDX12)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    add_executable(
        BasicGeometryFrameGraph
        MACOSX_BUNDLE
        ${SOURCE_FILES}
        ${forward_source_root}/macOS/MetalViewController.h
        ${forward_source_root}/macOS/MetalViewController.mm
        ${forward_source_root}/macOS/MainMenu.xib)

    target_link_libraries(BasicGeometryFrameGraph forwardMetal)

    set_source_files_properties(
        ${forward_source_root}/macOS/MainMenu.xib
        PROPERTIES
        MACOSX_PACKAGE_LOCATION
        Resources)

    set_target_properties(
        BasicGeometryFrameGraph
        PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        ${forward_source_root}/macOS/info.plist)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    -framework AppKit"
    )

    set_xcode_property(BasicGeometryFrameGraph "CLANG_CXX_LANGUAGE_STANDARD" "c++17")
    set_xcode_property(BasicGeometryFrameGraph "CLANG_CXX_LIBRARY" "libc++")
endif()

set_property(TARGET BasicGeometryFrameGraph PROPERTY CXX_STANDARD 17)
set_target_properties(BasicGeometryFrameGraph PROPERTIES FOLDER Application)