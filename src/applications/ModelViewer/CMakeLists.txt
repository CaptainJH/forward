
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(ModelViewer ${SOURCE_FILES})
target_link_libraries(ModelViewer libForward)
target_link_libraries(ModelViewer MaterialXReader)

if(forward_CompileWithSuperluminalAPI AND SuperluminalAPI_FOUND)
    target_link_libraries(ModelViewer SuperluminalAPI)
endif()

set_property(TARGET ModelViewer PROPERTY CXX_STANDARD 20)
set_target_properties(ModelViewer PROPERTIES FOLDER Application)