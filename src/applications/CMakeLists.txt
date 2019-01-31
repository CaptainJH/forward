cmake_minimum_required(VERSION 3.10)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_subdirectory(HelloWorld)
	add_subdirectory(BasicGeometryApp)
	add_subdirectory(MSAA)
	#add_subdirectory(ShadowMapDemo)
	#add_subdirectory(AssimpDemo)
	#add_subdirectory(SDFShadowDemo)
	add_subdirectory(SmartPtrDemo)
	#------DirectX 12 apps -----------
	#add_subdirectory(HelloDX12)
	#add_subdirectory(BasicGeometryDX12)
	#add_subdirectory(HelloFrameGraphDX12)
	#------FrameGraph apps -----------
	add_subdirectory(HelloFrameGraphDemo)
	add_subdirectory(BasicGeometryFrameGraph)
	add_subdirectory(OffScreenRenderingDemo)
	#------Dlls-----------------------
	add_subdirectory(forwardDX11_Dll)
	add_subdirectory(HelloPyQT)
endif()

add_subdirectory(CmdApp)