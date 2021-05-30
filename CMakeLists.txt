cmake_minimum_required(VERSION 3.9.2)

project(Guider VERSION 0.0.1 DESCRIPTION "Gui library." LANGUAGES CXX)

option(BUILD_EXAMPLES "Build examples" true)
option(BUILD_TOOLS "Build tools" true)
option(BUILD_TESTS "Build tests" false)
option(INTERNAL_PARSELIB "Use parselib from submodule" false)
option(DEV_BUILD "For guider development only" false)

if (INTERNAL_PARSELIB)
	add_subdirectory(ParseLib)
	add_library(parselib::xml ALIAS xml)
endif()

add_subdirectory(guider)
if (BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
if (BUILD_TOOLS)
	add_subdirectory(tools)
endif()