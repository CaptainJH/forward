
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(MSAA ${SOURCE_FILES})
target_link_libraries(MSAA libForward)

set_property(TARGET MSAA PROPERTY CXX_STANDARD 20)
set_target_properties(MSAA PROPERTIES FOLDER Application)