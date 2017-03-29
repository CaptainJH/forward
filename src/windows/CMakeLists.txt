cmake_minimum_required(VERSION 3.3)

# Core
file(GLOB HEADER_FILES_CORE "${forward_source_root}/core/*.h" "${forward_source_root}/core/*.cpp")
source_group(Core FILES ${HEADER_FILES_CORE})
list(APPEND HEADER_FILES ${HEADER_FILES_CORE})

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

# utilities
file(GLOB SOURCE_FILES_UTILITIES "${forward_source_root}/utilities/*.*")
source_group(Utilities FILES ${SOURCE_FILES_UTILITIES})
list(APPEND SOURCE_FILES ${SOURCE_FILES_UTILITIES})

# DXGI
file(GLOB DXGI_FILES "${forward_source_root}/windows/DXGI*.*")
list(APPEND SOURCE_FILES ${DXGI_FILES})
source_group(RenderSystem\\DXGI FILES ${DXGI_FILES})

# include
file(GLOB HEADER_FILES_INCLUDE "${forward_source_root}/include/*.h" "${forward_source_root}/include/*.inl")
list(APPEND HEADER_FILES ${HEADER_FILES_INCLUDE})
source_group(Common FILES ${HEADER_FILES_INCLUDE})

# DirectX
file(GLOB RENDERSYSTEM_HEADER_FILES "${forward_source_root}/windows/d3dUtil.h" "${forward_source_root}/windows/Log.h")
file(GLOB RENDERSYSTEM_SOURCE_FILES "${forward_source_root}/windows/d3dUtil.cpp" "${forward_source_root}/windows/Log.cpp")

add_subdirectory(dx11)