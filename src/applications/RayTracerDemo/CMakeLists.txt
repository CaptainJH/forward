
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(RayTracerDemo ${SOURCE_FILES})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_link_libraries(RayTracerDemo forwardDX12)
    target_link_libraries(RayTracerDemo assimp)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set_xcode_property(RayTracerDemo "CLANG_CXX_LANGUAGE_STANDARD" "c++17")
    set_xcode_property(RayTracerDemo "CLANG_CXX_LIBRARY" "libc++")
endif()
target_link_libraries(RayTracerDemo libForward)

set_property(TARGET RayTracerDemo PROPERTY CXX_STANDARD 20)
set_target_properties(RayTracerDemo PROPERTIES FOLDER Application)