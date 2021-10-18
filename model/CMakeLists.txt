add_library(model
    BoardModel.cpp
    Serialize.cpp
)
target_include_directories(model PUBLIC .)
target_link_libraries(model fmt)


add_subdirectory(test)
