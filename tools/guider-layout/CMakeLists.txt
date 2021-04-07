add_executable(guider-layout "")
target_sources(guider-layout
PRIVATE
	src/main.cpp
	
	src/app.cpp
	include/app.hpp
)

set_target_properties(guider-layout PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(guider-layout PRIVATE include)
target_link_libraries(guider-layout PRIVATE sfml_backend)
target_include_directories(guider-layout PRIVATE include ${CMAKE_CURRENT_BINARY_DIR}/include)
set_target_properties(guider-layout PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}//$(Configuration))
target_compile_definitions(guider-layout PRIVATE RESOURCE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/resources")

install(TARGETS guider-layout DESTINATION bin)
add_custom_command(TARGET guider-layout POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/"
        $<TARGET_FILE_DIR:guider-layout>/resources/)