cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(HelloPyQT SHARED ${SOURCE_FILES})
target_link_libraries(HelloPyQT forwardDX11)
target_link_libraries(HelloPyQT forwardDX12)

set_target_properties(HelloPyQT PROPERTIES FOLDER Application)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-DFORWARD_EXPORTS)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_custom_command(TARGET HelloPyQT POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy 
		$<TARGET_FILE:HelloPyQT>
		"${CMAKE_CURRENT_SOURCE_DIR}")
endif()