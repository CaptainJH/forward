cmake_minimum_required(VERSION 3.14)

set(UnityPath "" CACHE PATH "Specify the Unity path")
if(NOT "${UnityPath}" STREQUAL "")
	include_directories("${UnityPath}")
endif()

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(UnityPlugin SHARED ${SOURCE_FILES})
target_link_libraries(UnityPlugin forwardDX11)
target_link_libraries(UnityPlugin forwardDX12)

set_target_properties(UnityPlugin PROPERTIES FOLDER Application)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-DFORWARD_EXPORTS)