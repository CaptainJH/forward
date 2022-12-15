
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(BitmapFontCreator ${SOURCE_FILES})
target_link_libraries(BitmapFontCreator libForward)
set_target_properties(BitmapFontCreator PROPERTIES FOLDER Tools)
set_target_properties(BitmapFontCreator PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")