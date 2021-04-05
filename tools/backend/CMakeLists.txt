find_package(SFML REQUIRED COMPONENTS graphics system)
find_package(OpenGL REQUIRED)

add_library(tools_backend backend.cpp backend.hpp)

set_target_properties(tools_backend PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tools_backend PUBLIC ${OPENGL_gl_LIBRARY} sfml-graphics sfml-window sfml-system ${SFML_DEPENDENCIES} OpenGL)
target_link_libraries(tools_backend PUBLIC guider)
target_include_directories(tools_backend PUBLIC ${SFML_INCLUDE_DIR})
target_include_directories(tools_backend PUBLIC INTERFACE ./)
target_include_directories(tools_backend PRIVATE ./)