cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(HelloFrameGraph ${SOURCE_FILES})
target_link_libraries(HelloFrameGraph forwardDX11)
target_link_libraries(HelloFrameGraph forwardDX12)

set_target_properties(HelloFrameGraph PROPERTIES FOLDER Application)