add_library(model
    BoardModel.cpp
)
target_include_directories(model PUBLIC .)
add_subdirectory(test)
