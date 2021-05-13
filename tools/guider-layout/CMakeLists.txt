add_executable(guider-layout "")
target_sources(guider-layout
PRIVATE
	src/main.cpp
	
	src/app.cpp
	include/app.hpp
)

set_target_properties(guider-layout PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(guider-layout PRIVATE include)
target_link_libraries(guider-layout PRIVATE tools-backend)
target_include_directories(guider-layout PRIVATE include ${CMAKE_CURRENT_BINARY_DIR}/include)
set_target_properties(guider-layout PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}//$(Configuration))
set_target_properties(guider-layout PROPERTIES CXX_STANDARD 17)

if(DEV_BUILD)
	target_compile_definitions(guider-layout PRIVATE RESOURCE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
else()
	target_compile_definitions(guider-layout PRIVATE RESOURCE_FOLDER="./resources/")
endif()
install(TARGETS guider-layout DESTINATION bin)
if(NOT DEV_BUILD)
	add_custom_command(TARGET guider-layout POST_BUILD
    		COMMAND ${CMAKE_COMMAND} -E copy_directory
        	"${CMAKE_CURRENT_SOURCE_DIR}/resources/"
       	 	$<TARGET_FILE_DIR:guider-layout>/resources/)
endif()