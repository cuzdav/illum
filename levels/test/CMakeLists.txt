include(GoogleTest)
enable_testing()

add_library(leveltest-lib INTERFACE)
target_include_directories(leveltest-lib INTERFACE .)


file(GLOB files "*Test.cpp")
package_add_test(leveltest ${files})
target_link_libraries (leveltest levels solver model leveltest-lib fmt)
add_test(NAME leveltest COMMAND leveltest)


