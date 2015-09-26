cmake_minimum_required(VERSION 3.3)


# DX11
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ApplicationDX11.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ApplicationDX11.cpp")
list(APPEND HEADER_FILES d3dUtil.h)
list(APPEND SOURCE_FILES d3dUtil.cpp main.cpp)
source_group(RenderSystem FILES ${HEADER_FILES} ${SOURCE_FILES})


# Resource System
file(GLOB RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ResourceDX11.*")
source_group(RenderSystem\\ResourceSystem FILES ${RESOURCE_FILES})
list(APPEND SOURCE_FILES ${RESOURCE_FILES})

file(GLOB BUFFER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/*Buffer*.*")
source_group(RenderSystem\\ResourceSystem\\Buffer FILES ${BUFFER_FILES})
list(APPEND SOURCE_FILES ${BUFFER_FILES})

file(GLOB TEXTURE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/Texture*.*" "${CMAKE_CURRENT_SOURCE_DIR}/dx11/SwapChain*.*")
source_group(RenderSystem\\ResourceSystem\\Texture FILES ${TEXTURE_FILES})
list(APPEND SOURCE_FILES ${TEXTURE_FILES})

file(GLOB RESOURCE_VIEW_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/*View*.*")
list(REMOVE_ITEM RESOURCE_VIEW_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ViewPortDX11.h")
list(REMOVE_ITEM RESOURCE_VIEW_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ViewPortDX11.cpp")
source_group(RenderSystem\\ResourceSystem\\ResourceView FILES ${RESOURCE_VIEW_FILES})
list(APPEND SOURCE_FILES ${RESOURCE_VIEW_FILES})

file(GLOB STATE_OBJECT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/*StateConfigDX11.*" "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ViewPortDX11.*")
source_group(RenderSystem\\ResourceSystem\\StateObject FILES ${STATE_OBJECT_FILES})
list(APPEND SOURCE_FILES ${STATE_OBJECT_FILES})


# Core
list(APPEND HEADER_FILES ${forward_source_root}/core/types.h)
source_group(Core FILES ${forward_source_root}/core/types.h)
list(APPEND HEADER_FILES ${HEADER_FILES})


# math
file(GLOB HEADER_FILES_MATH "${forward_source_root}/math/*.h" "${forward_source_root}/math/*.inl")
file(GLOB SOURCE_FILES_MATH "${forward_source_root}/math/*.cpp")
source_group(Math FILES ${HEADER_FILES_MATH} ${SOURCE_FILES_MATH})
list(APPEND HEADER_FILES ${HEADER_FILES_MATH})
list(APPEND SOURCE_FILES ${SOURCE_FILES_MATH})


# geometry
file(GLOB HEADER_FILES_GEOMETRY "${forward_source_root}/geometry/*.h")
file(GLOB SOURCE_FILES_GEOMETRY "${forward_source_root}/geometry/*.cpp")
source_group(Geometry FILES ${HEADER_FILES_GEOMETRY} ${SOURCE_FILES_GEOMETRY})
list(APPEND HEADER_FILES ${HEADER_FILES_GEOMETRY})
list(APPEND SOURCE_FILES ${SOURCE_FILES_GEOMETRY})


# intersection
file(GLOB HEADER_FILES_INTERSECTION "${forward_source_root}/intersection/*.h")
file(GLOB SOURCE_FILES_INTERSECTION "${forward_source_root}/intersection/*.cpp")
source_group(Intersection FILES ${HEADER_FILES_INTERSECTION} ${SOURCE_FILES_INTERSECTION})
list(APPEND HEADER_FILES ${HEADER_FILES_INTERSECTION})
list(APPEND SOURCE_FILES ${SOURCE_FILES_INTERSECTION})


add_executable(forward ${HEADER_FILES} ${SOURCE_FILES})