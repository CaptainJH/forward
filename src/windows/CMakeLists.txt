cmake_minimum_required(VERSION 3.3)

# DX11
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ApplicationDX11.h" "${CMAKE_CURRENT_SOURCE_DIR}/dx11/RendererDX11.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ApplicationDX11.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/dx11/RendererDX11.cpp")
list(APPEND HEADER_FILES d3dUtil.h Log.h)
list(APPEND SOURCE_FILES d3dUtil.cpp Log.cpp)
source_group(RenderSystem FILES ${HEADER_FILES} ${SOURCE_FILES})

# DXGI
file(GLOB DXGI_FILES "${CMAKE_CURRENT_SOURCE_DIR}/DXGI*.*")
list(APPEND SOURCE_FILES ${DXGI_FILES})
source_group(RenderSystem\\DXGI FILES ${DXGI_FILES})

# include
file(GLOB HEADER_FILES_INCLUDE "${forward_source_root}/include/*.h" "${forward_source_root}/include/*.inl")
list(APPEND HEADER_FILES ${HEADER_FILES_INCLUDE})
source_group(Common FILES ${HEADER_FILES_INCLUDE})

# PipeLine System
file(GLOB PIPELINE_IA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/InputAssembler*DX11.*")
source_group(RenderSystem\\Pipeline\\Stages\\FixedStages\\InputAssembler FILES ${PIPELINE_IA_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_IA_FILES})

file(GLOB PIPELINE_RASTERIZER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/RasterizerStage*.*")
source_group(RenderSystem\\Pipeline\\Stages\\FixedStages\\Rasterizer FILES ${PIPELINE_RASTERIZER_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_RASTERIZER_FILES})

file(GLOB PIPELINE_SO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/StreamOutputStage*.*")
source_group(RenderSystem\\Pipeline\\Stages\\FixedStages\\StreamOutput FILES ${PIPELINE_SO_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_SO_FILES})

file(GLOB PIPELINE_OM_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/OutputMergerStage*.*")
source_group(RenderSystem\\Pipeline\\Stages\\FixedStages\\OutputMerger FILES ${PIPELINE_OM_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_OM_FILES})

file(GLOB PIPELINE_SHADERSTAGESTATE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ShaderStageStateDX11.*")
source_group(RenderSystem\\Pipeline\\Stages\\ProgrammableStages\\States FILES ${PIPELINE_SHADERSTAGESTATE_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_SHADERSTAGESTATE_FILES})


file(GLOB PIPELINE_SHADERPROGRAM_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/*ShaderDX11.*")
file(GLOB PIPELINE_SHADERFACTORY_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ShaderFactoryDX11.*")
list(APPEND PIPELINE_SHADERPROGRAM_FILES ${PIPELINE_SHADERFACTORY_FILES})
source_group(RenderSystem\\Pipeline\\Stages\\ProgrammableStages\\ShaderProgram FILES ${PIPELINE_SHADERPROGRAM_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_SHADERPROGRAM_FILES})

file(GLOB PIPELINE_SHADERSTAGE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/*ShaderStageDX11.*")
source_group(RenderSystem\\Pipeline\\Stages\\ProgrammableStages\\ShaderStage FILES ${PIPELINE_SHADERSTAGE_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_SHADERSTAGE_FILES})

file(GLOB PIPELINE_EXECUTORS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/GeometryDX11.*" )
file(GLOB PIPELINE_EXECUTORS_FILES2 "${CMAKE_CURRENT_SOURCE_DIR}/dx11/PipelineExecutorDX11.*")
file(GLOB PIPELINE_EXECUTORS_FILES_VERTEX "${CMAKE_CURRENT_SOURCE_DIR}/dx11/VertexElementDX11.*")
list(APPEND PIPELINE_EXECUTORS_FILES ${PIPELINE_EXECUTORS_FILES2} ${PIPELINE_EXECUTORS_FILES_VERTEX})
source_group(RenderSystem\\Pipeline\\Executors FILES ${PIPELINE_EXECUTORS_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_EXECUTORS_FILES})

file(GLOB PIPELINE_MANAGER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/PipelineManagerDX11.*")
source_group(RenderSystem\\Pipeline FILES ${PIPELINE_MANAGER_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_MANAGER_FILES})

# Resource System
file(GLOB RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ResourceDX11.*" "${CMAKE_CURRENT_SOURCE_DIR}/dx11/ResourceProxyDX11.*")
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

add_library(forward STATIC ${HEADER_FILES} ${SOURCE_FILES})
if(Boost_FOUND)
	target_link_libraries(forward ${Boost_LIBRARIES})
endif()