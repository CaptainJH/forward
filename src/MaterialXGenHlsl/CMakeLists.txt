
file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.c*")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

add_library(MaterialXGenHlsl ${materialx_source} ${materialx_headers})

add_definitions(-DMATERIALX_GENHLSL_EXPORTS)

#add_definitions(-DNOMINMAX -DDX12)
set_property(TARGET MaterialXGenHlsl PROPERTY CXX_STANDARD 20)
target_link_libraries(
    MaterialXGenHlsl 
    MaterialXGenShader
    MaterialXCore)