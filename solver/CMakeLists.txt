add_library(solver
    AnalysisBoard.cpp
    AnnotatedMove.cpp
    Hint.cpp
    PositionBoard.cpp
    Solver.cpp
    SpeculationContext.cpp
    trivial_moves.cpp
)
target_include_directories(solver PUBLIC .)
target_link_libraries(solver model fmt common)

if (NOT EMSCRIPTEN)
    add_subdirectory(test)
else()
endif(NOT EMSCRIPTEN)
