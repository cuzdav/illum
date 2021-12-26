cmake_minimum_required (VERSION 3.20.3)
project (Illuminate)

OPTION(DEBUGLOG "Enable debug logging" OFF)
OPTION(DEBUGPROFILE "Enable cell-visitor counters" OFF)
OPTION(ASAN "Build linking the address sanitizer" OFF)

include("setup.cmake")

add_library(common INTERFACE)
target_include_directories(common INTERFACE .)
target_link_libraries(common INTERFACE fmt)

if (EMSCRIPTEN)
set(CMAKE_CXX_FLAGS "-s ALLOW_MEMORY_GROWTH=1 -s USE_LIBPNG=1 -s MAX_WEBGL_VERSION=2 -s MIN_WEBGL_VERSION=2")
endif(EMSCRIPTEN)

add_subdirectory (model)
add_subdirectory (levels)
add_subdirectory (solver)
add_subdirectory (olc)
add_subdirectory (gui)
