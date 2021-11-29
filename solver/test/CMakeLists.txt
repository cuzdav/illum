include(GoogleTest)

file(GLOB files "*Test.cpp")
list(FILTER files EXCLUDE REGEX ".*/\.#.*")
package_add_test(solvertests ${files})
target_link_libraries (solvertests solver model modeltest-lib)
add_test(NAME solvertests COMMAND solvertests)



