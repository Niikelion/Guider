find_package(GLEW REQUIRED)
find_package(SFML REQUIRED network audio graphics window system)

add_executable(sfml_backend main.cpp backend.cpp backend.hpp)
set_target_properties(sfml_backend PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(sfml_backend PRIVATE GLEW::GLEW)
target_link_libraries(sfml_backend PRIVATE sfml-graphics ${SFML_DEPENDENCIES})

target_link_libraries(sfml_backend PRIVATE guider)

target_include_directories(sfml_backend PRIVATE ${SFML_INCLUDE_DIR})

target_include_directories(sfml_backend PRIVATE ./)