add_executable(basic-example src/main.cpp)

target_link_libraries(basic-example PRIVATE sfml-backend)
set_target_properties(basic-example PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}//$(Configuration)
	VS_JUST_MY_CODE_DEBUGGING ON
	FOLDER "examples"
)
target_compile_features(basic-example PRIVATE cxx_std_17)
target_compile_definitions(basic-example PRIVATE RESOURCE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/resources/")