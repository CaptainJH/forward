
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(ReferencePT ${SOURCE_FILES})
target_link_libraries(ReferencePT libForward)

set_property(TARGET ReferencePT PROPERTY CXX_STANDARD 20)
set_target_properties(ReferencePT PROPERTIES FOLDER Application)