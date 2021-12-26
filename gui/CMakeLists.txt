add_library(illum
    Illum.cpp
)
target_include_directories(illum PUBLIC .)
target_link_libraries(illum levels solver model fmt common olc)

add_executable(main
    illum_main.cpp
)

if (EMSCRIPTEN)
    set_target_properties(main PROPERTIES
       LINK_FLAGS "-s ALLOW_MEMORY_GROWTH=1 -s USE_LIBPNG=1 -s MAX_WEBGL_VERSION=2 -s MIN_WEBGL_VERSION=2"
       SUFFIX ".html"
    )
endif(EMSCRIPTEN)

target_link_libraries(main illum)


