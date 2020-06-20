set(sources
	src/base.cpp
	src/components.cpp
	src/containers.cpp
)

set(headers
	include/guider/base.hpp
	include/guider/components.hpp
	include/guider/containers.hpp
)

add_library(guider STATIC ${sources} ${headers})
target_include_directories(guider PUBLIC include)