cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(BasicGeometryFrameGraph ${SOURCE_FILES})
target_link_libraries(BasicGeometryFrameGraph forwardDX11)
target_link_libraries(BasicGeometryFrameGraph forwardDX12)

set_target_properties(BasicGeometryFrameGraph PROPERTIES FOLDER Application)