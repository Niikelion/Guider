add_executable(common-elements-example src/main.cpp)

target_link_libraries(common-elements-example PRIVATE sfml-backend)
set_target_properties(common-elements-example PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}//$(Configuration))
set_target_properties(common-elements-example PROPERTIES VS_JUST_MY_CODE_DEBUGGING ON)
set_target_properties(common-elements-example PROPERTIES FOLDER "examples")

target_compile_definitions(common-elements-example PRIVATE RESOURCE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/resources/")