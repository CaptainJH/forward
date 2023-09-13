
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(HelloRaytracing ${SOURCE_FILES})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_link_libraries(HelloRaytracing forwardDX12)
    target_link_libraries(HelloRaytracing assimp)
endif()
target_link_libraries(HelloRaytracing libForward)

set_property(TARGET HelloRaytracing PROPERTY CXX_STANDARD 20)
set_target_properties(HelloRaytracing PROPERTIES FOLDER Application)