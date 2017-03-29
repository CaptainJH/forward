cmake_minimum_required(VERSION 3.3)


# DX11
list(APPEND RENDERSYSTEM_HEADER_FILES "${forward_source_root}/windows/dx11/ApplicationDX11.h" "${forward_source_root}/windows/dx11/RendererDX11.h")
list(APPEND RENDERSYSTEM_SOURCE_FILES "${forward_source_root}/windows/dx11/ApplicationDX11.cpp" "${forward_source_root}/windows/dx11/RendererDX11.cpp")
source_group(RenderSystem FILES ${RENDERSYSTEM_HEADER_FILES} ${RENDERSYSTEM_SOURCE_FILES})
list(APPEND HEADER_FILES ${RENDERSYSTEM_HEADER_FILES})
list(APPEND SOURCE_FILES ${RENDERSYSTEM_SOURCE_FILES})

# PipeLine System
file(GLOB PIPELINE_IA_FILES "${forward_source_root}/windows/dx11/InputAssembler*DX11.*")
source_group(RenderSystem\\Pipeline\\Stages\\FixedStages\\InputAssembler FILES ${PIPELINE_IA_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_IA_FILES})

file(GLOB PIPELINE_RASTERIZER_FILES "${forward_source_root}/windows/dx11/RasterizerStage*.*")
source_group(RenderSystem\\Pipeline\\Stages\\FixedStages\\Rasterizer FILES ${PIPELINE_RASTERIZER_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_RASTERIZER_FILES})

file(GLOB PIPELINE_SO_FILES "${forward_source_root}/windows/dx11/StreamOutputStage*.*")
source_group(RenderSystem\\Pipeline\\Stages\\FixedStages\\StreamOutput FILES ${PIPELINE_SO_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_SO_FILES})

file(GLOB PIPELINE_OM_FILES "${forward_source_root}/windows/dx11/OutputMergerStage*.*")
source_group(RenderSystem\\Pipeline\\Stages\\FixedStages\\OutputMerger FILES ${PIPELINE_OM_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_OM_FILES})

file(GLOB PIPELINE_SHADERSTAGESTATE_FILES "${forward_source_root}/windows/dx11/ShaderStageStateDX11.*")
source_group(RenderSystem\\Pipeline\\Stages\\ProgrammableStages\\States FILES ${PIPELINE_SHADERSTAGESTATE_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_SHADERSTAGESTATE_FILES})


file(GLOB PIPELINE_SHADERPROGRAM_FILES "${forward_source_root}/windows/dx11/*ShaderDX11.*")
file(GLOB PIPELINE_SHADERFACTORY_FILES "${forward_source_root}/windows/dx11/ShaderFactoryDX11.*")
list(APPEND PIPELINE_SHADERPROGRAM_FILES ${PIPELINE_SHADERFACTORY_FILES})
source_group(RenderSystem\\Pipeline\\Stages\\ProgrammableStages\\ShaderProgram FILES ${PIPELINE_SHADERPROGRAM_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_SHADERPROGRAM_FILES})

file(GLOB PIPELINE_SHADERSTAGE_FILES "${forward_source_root}/windows/dx11/*ShaderStageDX11.*")
source_group(RenderSystem\\Pipeline\\Stages\\ProgrammableStages\\ShaderStage FILES ${PIPELINE_SHADERSTAGE_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_SHADERSTAGE_FILES})

file(GLOB PIPELINE_EXECUTORS_FILES "${forward_source_root}/windows/dx11/GeometryDX11.*" )
file(GLOB PIPELINE_EXECUTORS_FILES2 "${forward_source_root}/windows/dx11/PipelineExecutorDX11.*")
file(GLOB PIPELINE_EXECUTORS_FILES_VERTEX "${forward_source_root}/windows/dx11/VertexElementDX11.*")
list(APPEND PIPELINE_EXECUTORS_FILES ${PIPELINE_EXECUTORS_FILES2} ${PIPELINE_EXECUTORS_FILES_VERTEX})
source_group(RenderSystem\\Pipeline\\Executors FILES ${PIPELINE_EXECUTORS_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_EXECUTORS_FILES})

file(GLOB PIPELINE_MANAGER_FILES "${forward_source_root}/windows/dx11/PipelineManagerDX11.*")
source_group(RenderSystem\\Pipeline FILES ${PIPELINE_MANAGER_FILES})
list(APPEND SOURCE_FILES ${PIPELINE_MANAGER_FILES})

# Resource System
file(GLOB RESOURCE_FILES "${forward_source_root}/windows/dx11/ResourceDX11.*" "${forward_source_root}/windows/dx11/ResourceProxyDX11.*")
source_group(RenderSystem\\ResourceSystem FILES ${RESOURCE_FILES})
list(APPEND SOURCE_FILES ${RESOURCE_FILES})

file(GLOB BUFFER_FILES "${forward_source_root}/windows/dx11/*Buffer*.*")
source_group(RenderSystem\\ResourceSystem\\Buffer FILES ${BUFFER_FILES})
list(APPEND SOURCE_FILES ${BUFFER_FILES})

file(GLOB TEXTURE_FILES "${forward_source_root}/windows/dx11/Texture*.*" "${forward_source_root}/windows/dx11/SwapChain*.*")
source_group(RenderSystem\\ResourceSystem\\Texture FILES ${TEXTURE_FILES})
list(APPEND SOURCE_FILES ${TEXTURE_FILES})

file(GLOB RESOURCE_VIEW_FILES "${forward_source_root}/windows/dx11/*View*.*")
list(REMOVE_ITEM RESOURCE_VIEW_FILES "${forward_source_root}/windows/dx11/ViewPortDX11.h")
list(REMOVE_ITEM RESOURCE_VIEW_FILES "${forward_source_root}/windows/dx11/ViewPortDX11.cpp")
source_group(RenderSystem\\ResourceSystem\\ResourceView FILES ${RESOURCE_VIEW_FILES})
list(APPEND SOURCE_FILES ${RESOURCE_VIEW_FILES})

file(GLOB STATE_OBJECT_FILES "${forward_source_root}/windows/dx11/*StateConfigDX11.*" "${forward_source_root}/windows/dx11/ViewPortDX11.*")
source_group(RenderSystem\\ResourceSystem\\StateObject FILES ${STATE_OBJECT_FILES})
list(APPEND SOURCE_FILES ${STATE_OBJECT_FILES})

add_library(forward STATIC ${HEADER_FILES} ${SOURCE_FILES})