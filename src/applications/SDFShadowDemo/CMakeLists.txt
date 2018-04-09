cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(SDFShadowDemo ${SOURCE_FILES})
target_link_libraries(SDFShadowDemo forward)
target_link_libraries(SDFShadowDemo assimp-vc130-mt.lib)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX-")
	add_custom_command(TARGET SDFShadowDemo POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy 
		"${PROJECT_SOURCE_DIR}/libs/assimp/Win64/assimp-vc130-mt.dll" 
		$<TARGET_FILE_DIR:SDFShadowDemo>)
endif()