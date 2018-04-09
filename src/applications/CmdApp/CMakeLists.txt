cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(CmdApp ${SOURCE_FILES})
set_target_properties(CmdApp PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(CmdApp forwardDX11_Hieroglyph)
target_link_libraries(CmdApp assimp-vc130-mt.lib)

set_target_properties(CmdApp PROPERTIES FOLDER Application)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX-")
	add_custom_command(TARGET CmdApp POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy 
		"${PROJECT_SOURCE_DIR}/libs/assimp/Win64/assimp-vc130-mt.dll" 
		$<TARGET_FILE_DIR:CmdApp>)
endif()