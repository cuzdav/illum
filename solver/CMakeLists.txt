add_library(solver
    Solver.cpp
)
target_include_directories(solver PUBLIC .)
target_link_libraries(solver model fmt)

add_subdirectory(test)
