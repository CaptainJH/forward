
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(HelloVolumeRendering ${SOURCE_FILES})
set_target_properties(HelloVolumeRendering PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(HelloVolumeRendering libForward TBB::tbb)
target_link_libraries(HelloVolumeRendering benchmark::benchmark)

set_property(TARGET HelloVolumeRendering PROPERTY CXX_STANDARD 20)
set_target_properties(HelloVolumeRendering PROPERTIES FOLDER Application)