if (NOT INTERNAL_PARSELIB)
	find_package(parselib CONFIG REQUIRED)
endif()
set(sources
	src/base.cpp
	src/manager.cpp
	src/styles.cpp
	src/components.cpp
	src/containers.cpp
)

set(headers
	include/guider/base.hpp
	include/guider/manager.hpp
	include/guider/components.hpp
	include/guider/containers.hpp
	include/guider/styles.hpp

	include/guider/shortcuts.hpp
)

add_library(guider STATIC ${sources} ${headers})
target_include_directories(guider PUBLIC include)
target_link_libraries(guider PUBLIC parselib::xml)
set_target_properties(guider PROPERTIES VS_JUST_MY_CODE_DEBUGGING ON)