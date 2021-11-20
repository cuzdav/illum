add_library(solver
    AnalysisBoard.cpp
    PositionBoard.cpp
    Solver.cpp
    trivial/trivial_moves.cpp
)
target_include_directories(solver PUBLIC .)
target_link_libraries(solver model fmt illum)

add_subdirectory(test)
