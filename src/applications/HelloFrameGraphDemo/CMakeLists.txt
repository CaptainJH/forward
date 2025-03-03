
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_executable(HelloFrameGraph ${SOURCE_FILES})
    target_link_libraries(HelloFrameGraph libForward)
    target_link_libraries(HelloFrameGraph assimp)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    add_executable(
        HelloFrameGraph
        MACOSX_BUNDLE
        ${SOURCE_FILES}
        ${forward_source_root}/macOS/MetalViewController.h
        ${forward_source_root}/macOS/MetalViewController.mm
        ${forward_source_root}/macOS/MainMenu.xib)

    target_link_libraries(HelloFrameGraph forwardMetal)

    set_source_files_properties(
        ${forward_source_root}/macOS/MainMenu.xib
        PROPERTIES
        MACOSX_PACKAGE_LOCATION
        Resources)

    set_target_properties(
        HelloFrameGraph
        PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        ${forward_source_root}/macOS/info.plist)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    -framework AppKit"
    )

    set_xcode_property(HelloFrameGraph "CLANG_CXX_LANGUAGE_STANDARD" "c++17")
    set_xcode_property(HelloFrameGraph "CLANG_CXX_LIBRARY" "libc++")
endif()

set_property(TARGET HelloFrameGraph PROPERTY CXX_STANDARD 20)
set_target_properties(HelloFrameGraph PROPERTIES FOLDER Application)