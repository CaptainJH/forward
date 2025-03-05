
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(SH ${SOURCE_FILES})
target_link_libraries(SH libForward)

set_property(TARGET SH PROPERTY CXX_STANDARD 20)
set_target_properties(SH PROPERTIES FOLDER Application)