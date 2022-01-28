cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(HelloDX12 ${SOURCE_FILES})
target_link_libraries(HelloDX12 forwardDX11)
target_link_libraries(HelloDX12 forwardDX12)

set_property(TARGET HelloDX12 PROPERTY CXX_STANDARD 17)
set_target_properties(HelloDX12 PROPERTIES FOLDER Application)