option(SFML_BACKEND "Allow building sfml backend" true)

if (SFML_BACKEND)
	add_subdirectory(sfml)
endif()