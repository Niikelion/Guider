find_package(parselib REQUIRED)
find_package(skia REQUIRED)

set(sources
	src/base.cpp
	src/components.cpp
	src/containers.cpp
	src/manager.cpp
)

set(headers
	include/guider/base.hpp
	include/guider/components.hpp
	include/guider/containers.hpp
	include/guider/manager.hpp
)

add_library(guider STATIC ${sources} ${headers})
target_include_directories(guider PUBLIC include)
target_link_libraries(guider PUBLIC parselib::xml skia skia::skia)