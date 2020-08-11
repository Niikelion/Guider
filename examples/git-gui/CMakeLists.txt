SET(GIT_GUI_SOURCES
	src/main.cpp
	src/gui.cpp
	src/git.cpp
)
SET(GIT_GUI_HEADERS
	include/gui.hpp
	include/git.hpp
)

add_executable(git-gui ${GIT_GUI_SOURCES} ${GIT_GUI_HEADERS})

set_target_properties(git-gui PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(git-gui PRIVATE sfml_backend)
target_include_directories(git-gui PRIVATE include)
set_target_properties(git-gui PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}//$(Configuration))

install(TARGETS git-gui DESTINATION bin)
add_custom_command(TARGET git-gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/"
        $<TARGET_FILE_DIR:git-gui>/resources/)