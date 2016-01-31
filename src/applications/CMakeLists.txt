cmake_minimum_required(VERSION 3.3)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_subdirectory(HelloWorld)
	add_subdirectory(BasicGeometryApp)
	add_subdirectory(MSAA)
	add_subdirectory(ShadowMapDemo)
	add_subdirectory(AssimpDemo)
	add_subdirectory(SDFShadowDemo)
endif()

add_subdirectory(CmdApp)