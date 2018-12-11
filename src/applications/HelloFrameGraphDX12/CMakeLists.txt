cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(HelloFrameGraphDX12 ${SOURCE_FILES})
target_link_libraries(HelloFrameGraphDX12 forwardDX12)

set_target_properties(HelloFrameGraphDX12 PROPERTIES FOLDER Application)