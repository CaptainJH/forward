cmake_minimum_required(VERSION 3.3)

include_directories("${forward_source_root}/core/")

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/application/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/application/*.cpp")

list(APPEND HEADER_FILES ${forward_source_root}/core/types.h)


add_executable(forward ${HEADER_FILES} ${SOURCE_FILES})