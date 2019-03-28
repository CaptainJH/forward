cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(MSAA ${SOURCE_FILES})
target_link_libraries(MSAA forwardDX11)
target_link_libraries(MSAA forwardDX12)

set_target_properties(MSAA PROPERTIES FOLDER Application)