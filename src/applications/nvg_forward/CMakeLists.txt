
file(GLOB SOURCE_FILES 
		"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/*.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(nvg_forward ${SOURCE_FILES})
target_link_libraries(nvg_forward libForward)

set_property(TARGET nvg_forward PROPERTY CXX_STANDARD 20)
set_target_properties(nvg_forward PROPERTIES FOLDER Application)