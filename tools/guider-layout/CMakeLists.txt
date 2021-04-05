SET(LAYOUT_SOURCES
	src/main.cpp
)
SET(LAYOUT_HEADERS
)

add_executable(guider-layout ${LAYOUT_SOURCES} ${LAYOUT_HEADERS})

set_target_properties(guider-layout PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(guider-layout PRIVATE sfml_backend)
target_include_directories(guider-layout PRIVATE include ${CMAKE_CURRENT_BINARY_DIR}/include)
set_target_properties(guider-layout PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}//$(Configuration))

install(TARGETS guider-layout DESTINATION bin)
add_custom_command(TARGET guider-layout POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/"
        $<TARGET_FILE_DIR:guider-layout>/resources/)