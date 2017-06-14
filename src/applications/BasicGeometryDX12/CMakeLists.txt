cmake_minimum_required(VERSION 3.3)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(BasicGeometryDX12 ${SOURCE_FILES})
target_link_libraries(BasicGeometryDX12 forward_dx12)