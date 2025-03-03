
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(TexLoadingTest ${SOURCE_FILES})
set_target_properties(TexLoadingTest PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(TexLoadingTest libForward)
target_link_libraries(TexLoadingTest assimp)

set_property(TARGET TexLoadingTest PROPERTY CXX_STANDARD 20)
set_target_properties(TexLoadingTest PROPERTIES FOLDER Application)