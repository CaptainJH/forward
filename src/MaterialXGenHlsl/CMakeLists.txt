
find_source_files(materialx_gen_hlsl_SOURCE_FILES "${forward_source_root}/MaterialXGenHlsl")
set_source_groups("${materialx_gen_hlsl_SOURCE_FILES}")

add_library(MaterialXGenHlsl ${materialx_gen_hlsl_SOURCE_FILES})

add_definitions(-DMATERIALX_GENHLSL_EXPORTS)

#add_definitions(-DNOMINMAX -DDX12)
set_property(TARGET MaterialXGenHlsl PROPERTY CXX_STANDARD 20)
target_link_libraries(
    MaterialXGenHlsl 
    MaterialXGenShader
    MaterialXCore)