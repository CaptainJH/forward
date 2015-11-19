cmake_minimum_required(VERSION 3.3)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(MSAA ${SOURCE_FILES})
target_link_libraries(MSAA forward)