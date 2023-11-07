
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_executable(SkyRenderingCPU ${SOURCE_FILES})
    set_target_properties(SkyRenderingCPU PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    target_link_libraries(SkyRenderingCPU forwardDX12)

    if(forward_CompileWithSuperluminalAPI AND SuperluminalAPI_FOUND)
        target_link_libraries(SkyRenderingCPU SuperluminalAPI)
    endif()
endif()

set_property(TARGET SkyRenderingCPU PROPERTY CXX_STANDARD 20)
set_target_properties(SkyRenderingCPU PROPERTIES FOLDER Application)