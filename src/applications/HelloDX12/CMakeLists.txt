
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(HelloDX12 ${SOURCE_FILES})
target_link_libraries(HelloDX12 forwardDX12)    
target_link_libraries(HelloDX12 assimp)

set_property(TARGET HelloDX12 PROPERTY CXX_STANDARD 20)
set_target_properties(HelloDX12 PROPERTIES FOLDER Application)

#add_custom_command(TARGET HelloDX12 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy 
#	$<TARGET_FILE:assimp> $<TARGET_FILE_DIR:HelloDX12>)