add_library(model
    BasicBoard.cpp
    BoardModel.cpp
    Coord.cpp
    Serialize.cpp
    SingleMove.cpp
)
target_include_directories(model PUBLIC .)
target_link_libraries(model fmt illum)


add_subdirectory(test)
