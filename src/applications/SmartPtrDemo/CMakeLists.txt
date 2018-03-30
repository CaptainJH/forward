cmake_minimum_required(VERSION 3.3)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(SmartPtrDemo ${SOURCE_FILES})
set_target_properties(SmartPtrDemo PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(SmartPtrDemo libForward)