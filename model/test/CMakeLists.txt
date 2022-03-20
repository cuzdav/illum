include(GoogleTest)
enable_testing()

add_library(modeltest-lib INTERFACE)
target_include_directories(modeltest-lib INTERFACE .)


file(GLOB files "*Test.cpp")
package_add_test(boardmodeltest ${files})
target_link_libraries (boardmodeltest model modeltest-lib fmt picojson)
add_test(NAME boardmodeltest COMMAND boardmodeltest)


