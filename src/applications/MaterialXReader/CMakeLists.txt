cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(MaterialXReader SHARED ${SOURCE_FILES})
target_link_libraries(MaterialXReader MaterialXFormat MaterialXGenGlsl MaterialXGenHlsl MaterialXRender)

set_target_properties(MaterialXReader PROPERTIES FOLDER Application)
set_target_properties(MaterialXReader PROPERTIES CXX_STANDARD 20)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-DFORWARD_EXPORTS)