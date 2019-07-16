cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(RayTracerDemo ${SOURCE_FILES})
set_target_properties(RayTracerDemo PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(RayTracerDemo libForward)

set_target_properties(RayTracerDemo PROPERTIES FOLDER Application)