
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(PBRLUTBaker ${SOURCE_FILES})
set_target_properties(PBRLUTBaker PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(PBRLUTBaker forwardDX12)
target_link_libraries(PBRLUTBaker assimp)

set_property(TARGET PBRLUTBaker PROPERTY CXX_STANDARD 20)
set_target_properties(PBRLUTBaker PROPERTIES FOLDER Application)