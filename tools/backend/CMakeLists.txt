find_package(SFML REQUIRED COMPONENTS graphics system)
find_package(OpenGL REQUIRED)

add_library(tools-backend backend.cpp backend.hpp)

set_target_properties(tools-backend PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tools-backend PUBLIC ${OPENGL_gl_LIBRARY} sfml-graphics sfml-window sfml-system ${SFML_DEPENDENCIES})
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_link_libraries(tools-backend PUBLIC OpenGL::GL OpenGL::GLU OpenGL::GLX)
endif()
target_link_libraries(tools-backend PUBLIC guider)
target_include_directories(tools-backend PUBLIC ${SFML_INCLUDE_DIR})
target_include_directories(tools-backend PUBLIC ./)