include(GoogleTest)
enable_testing()

add_library(utilstest-lib INTERFACE)
target_include_directories(utilstest-lib INTERFACE ${PROJECT_SOURCE_DIR})


file(GLOB files "*Test.cpp")
package_add_test(utilstest ${files})
target_link_libraries (utilstest utils utilstest-lib fmt)
add_test(NAME utilstest COMMAND utilstest)


