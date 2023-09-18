
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(RaytracingSimpleLighting ${SOURCE_FILES})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_link_libraries(RaytracingSimpleLighting forwardDX12)
    target_link_libraries(RaytracingSimpleLighting assimp)
endif()
target_link_libraries(RaytracingSimpleLighting libForward)

set_property(TARGET RaytracingSimpleLighting PROPERTY CXX_STANDARD 20)
set_target_properties(RaytracingSimpleLighting PROPERTIES FOLDER Application)