
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_executable(DXR_Tutorial_3 ${SOURCE_FILES})
    target_link_libraries(DXR_Tutorial_3 libForward)

    if(forward_CompileWithSuperluminalAPI AND SuperluminalAPI_FOUND)
        target_link_libraries(DXR_Tutorial_3 SuperluminalAPI)
    endif()
endif()

set_property(TARGET DXR_Tutorial_3 PROPERTY CXX_STANDARD 20)
set_target_properties(DXR_Tutorial_3 PROPERTIES FOLDER Application)