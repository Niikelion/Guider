find_package(GTest CONFIG REQUIRED)
add_executable(core-tests main.cpp core.cpp)
target_link_libraries(core-tests PRIVATE GTest::gtest guider)
set_target_properties(core-tests PROPERTIES CXX_STANDARD 17)

set_target_properties(core-tests PROPERTIES FOLDER "tests")