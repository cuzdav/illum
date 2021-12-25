add_library(model
    BasicBoard.cpp
    BoardModel.cpp
    Coord.cpp
    Direction.cpp
    Serialize.cpp
    SingleMove.cpp
)
target_include_directories(model PUBLIC .)
target_link_libraries(model fmt common)

if (NOT EMSCRIPTEN)
    add_subdirectory(test)
endif(NOT EMSCRIPTEN)
