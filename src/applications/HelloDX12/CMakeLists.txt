cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(HelloDX12 ${SOURCE_FILES})
target_link_libraries(HelloDX12 forward_dx12)