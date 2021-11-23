#pragma once
#include "BasicBoard.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include <optional>
#include <variant>

namespace solver {

struct IsolatedMarkCoordinate {
  model::Coord coord_;
};

using OptCoord         = model::OptCoord;
using OptAnnotatedMove = std::optional<AnnotatedMove>;
using AnnotatedMoves   = std::vector<AnnotatedMove>;

using IsolatedCell =
    std::variant<std::monostate, AnnotatedMove, IsolatedMarkCoordinate>;

IsolatedCell find_isolated_cell(model::BasicBoard const & board);

OptCoord find_wall_with_deps_equal_open_faces(model::BasicBoard const & board);

OptCoord find_satisfied_wall_having_open_faces(model::BasicBoard const & board);

// ------------- Find "all" such moves into a vector -----------
// returns true if board is in good shape, false if there's a contradiction
bool find_isolated_cells(model::BasicBoard const & board,
                         AnnotatedMoves &          moves);

void find_satisfied_walls_having_open_faces(model::BasicBoard const & board,
                                            AnnotatedMoves &          moves);

void find_walls_with_deps_equal_open_faces(model::BasicBoard const & board,
                                           AnnotatedMoves &          moves);

// one-stop shopping for isolated cells, satisfied walls, and walls that can
// be satisfied with the same number of bulbs as open faces. While it does not
// expressly validate the board, it may detect a contradiction and return
// false. If it did not detect a contradiction, it returns true. (found a mark
// that cannot be illuminated)
bool find_trivial_moves(model::BasicBoard const & board,
                        AnnotatedMoves &          moves);

// ------------ play one move

// both trivial and forced moves
bool enqueue_any_forced_move(Solution & solution);

// trivial marks are forced, but so trivial they don't count for speculation
// depth.  Play them all.
bool enqueue_trivial_marks(Solution & solution);

// a simple forced move, but is counted for speculation depth.
bool enqueue_forced_move(Solution & solution);

} // namespace solver
