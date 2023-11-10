
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(VolumeViewerGPU ${SOURCE_FILES})
target_link_libraries(VolumeViewerGPU forwardDX12)
target_link_libraries(VolumeViewerGPU openvdb_shared)

if(forward_CompileWithSuperluminalAPI AND SuperluminalAPI_FOUND)
    target_link_libraries(VolumeViewerGPU SuperluminalAPI)
endif()

set_property(TARGET VolumeViewerGPU PROPERTY CXX_STANDARD 20)
set_target_properties(VolumeViewerGPU PROPERTIES FOLDER Application)