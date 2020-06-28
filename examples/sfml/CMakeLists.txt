find_package(SFML REQUIRED network audio graphics window system)
find_package(OpenGL REQUIRED)

add_executable(sfml_backend main.cpp backend.cpp backend.hpp)
set_target_properties(sfml_backend PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(sfml_backend PRIVATE ${OPENGL_gl_LIBRARY} sfml-graphics sfml-window sfml-system ${SFML_DEPENDENCIES} OpenGL)

target_link_libraries(sfml_backend PRIVATE guider)

target_include_directories(sfml_backend PRIVATE ${SFML_INCLUDE_DIR})

target_include_directories(sfml_backend PRIVATE ./)