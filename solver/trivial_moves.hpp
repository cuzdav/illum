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

// returns an optional coordinate:
// empty: no error detected
// has_value: location of (invalid) mark that cannot be illuminated

OptCoord find_isolated_cells(model::BasicBoard const & board,
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
// the location of a mark that cannot be illuminated.
OptCoord find_trivial_moves(model::BasicBoard const & board,
                            AnnotatedMoves &          moves);

} // namespace solver
