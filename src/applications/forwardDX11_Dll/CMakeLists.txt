cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(forwardDX11_Dll SHARED ${SOURCE_FILES})
target_link_libraries(forwardDX11_Dll forwardDX11)

set_target_properties(forwardDX11_Dll PROPERTIES FOLDER Application)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-DFORWARD_EXPORTS)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_custom_command(TARGET forwardDX11_Dll POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy 
		$<TARGET_FILE:forwardDX11_Dll>
		"${PROJECT_SOURCE_DIR}/libs/")
endif()