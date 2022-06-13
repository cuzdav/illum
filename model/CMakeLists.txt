SET(MODEL_FILES 
    BasicBoard.cpp
    BoardModel.cpp
    Coord.cpp
    Direction.cpp
    SingleMove.cpp
)

add_library(model
   ${MODEL_FILES}
)
target_include_directories(model PUBLIC .)
target_link_libraries(model fmt common picojson)

if (EMSCRIPTEN)
else()
    add_subdirectory(test)
endif(EMSCRIPTEN)
