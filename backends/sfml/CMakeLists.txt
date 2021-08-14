find_package(SFML REQUIRED COMPONENTS graphics system)
find_package(OpenGL REQUIRED)

add_library(sfml-backend backend.cpp backend.hpp)

set_target_properties(sfml-backend PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(sfml-backend PROPERTIES FOLDER "backends")

target_link_libraries(sfml-backend PUBLIC ${OPENGL_gl_LIBRARY} sfml-graphics sfml-window sfml-system ${SFML_DEPENDENCIES})
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_link_libraries(sfml-backend PUBLIC OpenGL::GL OpenGL::GLX)
endif()
target_link_libraries(sfml-backend PUBLIC guider)
target_include_directories(sfml-backend PUBLIC ${SFML_INCLUDE_DIR} ./)