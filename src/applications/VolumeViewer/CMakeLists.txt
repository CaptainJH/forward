
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(VolumeViewer ${SOURCE_FILES})
set_target_properties(VolumeViewer PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(VolumeViewer forwardDX12)
target_link_libraries(VolumeViewer openvdb_shared)

if(forward_CompileWithSuperluminalAPI AND SuperluminalAPI_FOUND)
    target_link_libraries(VolumeViewer SuperluminalAPI)
endif()

set_property(TARGET VolumeViewer PROPERTY CXX_STANDARD 20)
set_target_properties(VolumeViewer PROPERTIES FOLDER Application)