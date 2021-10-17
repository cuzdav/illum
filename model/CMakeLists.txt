add_library(model
    BoardModel.cpp
)
target_include_directories(model PUBLIC .)
target_link_libraries(model olc)

add_subdirectory(test)
