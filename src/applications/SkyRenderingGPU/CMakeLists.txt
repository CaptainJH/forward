
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_executable(SkyRenderingGPU ${SOURCE_FILES})
    target_link_libraries(SkyRenderingGPU forwardDX12)

    if(forward_CompileWithSuperluminalAPI AND SuperluminalAPI_FOUND)
        target_link_libraries(SkyRenderingGPU SuperluminalAPI)
    endif()
endif()

set_property(TARGET SkyRenderingGPU PROPERTY CXX_STANDARD 20)
set_target_properties(SkyRenderingGPU PROPERTIES FOLDER Application)