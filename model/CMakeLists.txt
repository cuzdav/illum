SET(MODEL_FILES 
    BasicBoard.cpp
    BoardModel.cpp
    Coord.cpp
    Direction.cpp
    SingleMove.cpp
)

if (EMSCRIPTEN) 
else ()
list(APPEND MODEL_FILES Serialize.cpp)
endif(EMSCRIPTEN)


add_library(model
   ${MODEL_FILES}
)
target_include_directories(model PUBLIC .)
target_link_libraries(model fmt common)

if (EMSCRIPTEN)
else()
    add_subdirectory(test)
endif(EMSCRIPTEN)
