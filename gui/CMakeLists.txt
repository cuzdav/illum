add_library(illum
    Illum.cpp
)
target_include_directories(illum PUBLIC .)
target_link_libraries(illum levels solver model fmt common olc)

add_executable(main
    illum_main.cpp
)
target_link_libraries(main illum)


