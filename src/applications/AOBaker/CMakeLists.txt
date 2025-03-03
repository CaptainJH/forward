
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(AOBaker ${SOURCE_FILES})
target_link_libraries(AOBaker libForward)

set_property(TARGET AOBaker PROPERTY CXX_STANDARD 20)
set_target_properties(AOBaker PROPERTIES FOLDER Application)