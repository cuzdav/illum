cmake_minimum_required (VERSION 3.20.3)
project (Illuminate)

OPTION(DEBUGLOG "Enable debug logging" OFF)
OPTION(DEBUGPROFILE "Enable cell-visitor counters" OFF)
OPTION(ASAN "Build linking the address sanitizer" OFF)

include("setup.cmake")

add_library(common INTERFACE)
target_include_directories(common INTERFACE .)
target_link_libraries(common INTERFACE fmt)

add_subdirectory (model)
add_subdirectory (levels)
add_subdirectory (solver)
add_subdirectory (olc)
add_subdirectory (gui)
