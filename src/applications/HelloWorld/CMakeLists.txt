cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(HelloWorld ${SOURCE_FILES})
target_link_libraries(HelloWorld forwardDX11_Hieroglyph)

set_target_properties(HelloWorld PROPERTIES FOLDER Application)