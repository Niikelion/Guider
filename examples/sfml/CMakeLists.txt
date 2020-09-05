find_package(SFML REQUIRED COMPONENTS network audio graphics window system)
find_package(OpenGL REQUIRED)

add_library(sfml_backend backend.cpp backend.hpp)

add_executable(sfml_backend_test main.cpp)

set_target_properties(sfml_backend PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(sfml_backend PUBLIC ${OPENGL_gl_LIBRARY} sfml-graphics sfml-window sfml-system ${SFML_DEPENDENCIES} OpenGL)
target_link_libraries(sfml_backend PUBLIC guider)
target_include_directories(sfml_backend PUBLIC ${SFML_INCLUDE_DIR})
target_include_directories(sfml_backend PUBLIC INTERFACE ./)
target_include_directories(sfml_backend PRIVATE ./)

set_target_properties(sfml_backend_test PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(sfml_backend_test PRIVATE sfml_backend)
target_include_directories(sfml_backend_test PRIVATE ./)