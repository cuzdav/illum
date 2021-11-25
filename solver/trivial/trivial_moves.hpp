#pragma once
#include "BasicBoard.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include <optional>

namespace solver {

using OptCoord         = model::OptCoord;
using OptAnnotatedMove = std::optional<AnnotatedMove>;
using AnnotatedMoves   = std::vector<AnnotatedMove>;

// returns true if board is in good shape, false if there's a contradiction
bool find_isolated_cells(model::BasicBoard const & board,
                         AnnotatedMoves &          moves);

// returns moves to illuminate empty spaces around satisfied walls
void find_satisfied_walls_having_open_faces(model::BasicBoard const & board,
                                            AnnotatedMoves &          moves);

// returns moves to add bulbs around walls where all open faces must contain
// bulbs
void find_walls_with_deps_equal_open_faces(model::BasicBoard const & board,
                                           AnnotatedMoves &          moves);

// one-stop shopping for isolated cells, satisfied walls, and walls that can
// be satisfied with the same number of bulbs as open faces. While it does not
// expressly validate the board, it may detect a contradiction and return
// false. If it did not detect a contradiction, it returns true. (found a mark
// that cannot be illuminated)
bool find_trivial_moves(model::BasicBoard const & board,
                        AnnotatedMoves &          moves);

} // namespace solver
