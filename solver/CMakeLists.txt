add_library(solver
    AnalysisBoard.cpp
    AnnotatedMove.cpp
    PositionBoard.cpp
    Solver.cpp
    SpeculationContext.cpp
    trivial_moves.cpp
)
target_include_directories(solver PUBLIC .)
target_link_libraries(solver model fmt common)

add_subdirectory(test)
