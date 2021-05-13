cmake_minimum_required(VERSION 3.9.2)

project(Guider VERSION 0.0.1 DESCRIPTION "Gui library." LANGUAGES CXX)

add_subdirectory(guider)
if (BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
if (BUILD_TOOLS)
	add_subdirectory(tools)
endif()