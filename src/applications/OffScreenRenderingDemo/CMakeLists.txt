cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(OffScreenRenderingDemo ${SOURCE_FILES})
set_target_properties(OffScreenRenderingDemo PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(OffScreenRenderingDemo forwardDX11)
target_link_libraries(OffScreenRenderingDemo forwardDX12)

set_target_properties(OffScreenRenderingDemo PROPERTIES FOLDER Application)