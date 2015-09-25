cmake_minimum_required(VERSION 3.3)

include_directories("${forward_source_root}/core/")
include_directories("${forward_source_root}/windows/")
include_directories("${forward_source_root}/math/")
include_directories("${forward_source_root}/intersection/")
include_directories("${forward_source_root}/geometry/")


file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/*.cpp")
list(APPEND HEADER_FILES d3dUtil.h)
list(APPEND SOURCE_FILES d3dUtil.cpp)

source_group(RenderSystem FILES ${HEADER_FILES} ${SOURCE_FILES})

list(APPEND HEADER_FILES ${forward_source_root}/core/types.h)

source_group(Core FILES ${forward_source_root}/core/types.h)

file(GLOB HEADER_FILES_MATH "${forward_source_root}/math/*.h" "${forward_source_root}/math/*.inl")
file(GLOB SOURCE_FILES_MATH "${forward_source_root}/math/*.cpp")
source_group(Math FILES ${HEADER_FILES_MATH} ${SOURCE_FILES_MATH})


file(GLOB HEADER_FILES_GEOMETRY "${forward_source_root}/geometry/*.h")
file(GLOB SOURCE_FILES_GEOMETRY "${forward_source_root}/geometry/*.cpp")
source_group(Geometry FILES ${HEADER_FILES_GEOMETRY} ${SOURCE_FILES_GEOMETRY})

file(GLOB HEADER_FILES_INTERSECTION "${forward_source_root}/intersection/*.h")
file(GLOB SOURCE_FILES_INTERSECTION "${forward_source_root}/intersection/*.cpp")
source_group(Intersection FILES ${HEADER_FILES_INTERSECTION} ${SOURCE_FILES_INTERSECTION})

#MESSAGE(${HEADRE_FILES})


add_executable(forward ${HEADER_FILES_MATH} ${SOURCE_FILES_MATH} ${HEADER_FILES_INTERSECTION} ${SOURCE_FILES_INTERSECTION} ${HEADER_FILES_GEOMETRY} ${SOURCE_FILES_GEOMETRY} ${HEADER_FILES} ${SOURCE_FILES})