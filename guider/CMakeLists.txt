if (NOT INTERNAL_PARSELIB)
	find_package(parselib CONFIG REQUIRED)
endif()
set(sources
	src/definitions.cpp
	src/base.cpp
	src/manager.cpp
	src/styles.cpp
	src/parsing.cpp
	src/parsing/xml.cpp
	src/parsing/json.cpp
	src/components.cpp
	src/containers.cpp
)

set(headers
	include/guider/definitions.hpp
	include/guider/base.hpp
	include/guider/manager.hpp
	include/guider/parsing.hpp
	include/guider/parsing/xml.hpp
	include/guider/parsing/json.hpp
	include/guider/components.hpp
	include/guider/containers.hpp
	include/guider/styles.hpp

	include/guider/shortcuts.hpp
)

add_library(guider STATIC ${sources} ${headers})
target_include_directories(guider PUBLIC include)
target_link_libraries(guider PUBLIC parselib::xml)
set_target_properties(guider PROPERTIES CXX_STANDARD 17)
set_target_properties(guider PROPERTIES VS_JUST_MY_CODE_DEBUGGING ON)

if(BUILD_TESTS)
	add_subdirectory(tests)
endif()