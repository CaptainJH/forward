
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_executable(DXR_Tutorial_4_2 ${SOURCE_FILES})
    target_link_libraries(DXR_Tutorial_4_2 forwardDX12)

    if(forward_CompileWithSuperluminalAPI AND SuperluminalAPI_FOUND)
        target_link_libraries(DXR_Tutorial_4_2 SuperluminalAPI)
    endif()
endif()

set_property(TARGET DXR_Tutorial_4_2 PROPERTY CXX_STANDARD 20)
set_target_properties(DXR_Tutorial_4_2 PROPERTIES FOLDER Application)