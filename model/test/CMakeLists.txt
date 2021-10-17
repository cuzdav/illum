include(GoogleTest)
enable_testing()

file(GLOB files "*.cpp")
package_add_test(boardmodeltest ${files})
target_link_libraries (boardmodeltest model fmt)



