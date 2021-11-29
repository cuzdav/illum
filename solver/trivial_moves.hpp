#pragma once
#include "AnnotatedMove.hpp"
#include "BasicBoard.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
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

struct BoardAnalysis {
  std::vector<model::Coord> walls_with_deps;
};

std::unique_ptr<BoardAnalysis>
create_board_analysis(model::BasicBoard const & board);

// returns moves to add bulbs around walls where all open faces must contain
// bulbs, and corner marks where a bulb would leave wall unsatisfiable.
void find_around_walls_with_deps(model::BasicBoard const & board,
                                 BoardAnalysis const *     context,
                                 AnnotatedMoves &          moves);

// If multiple cells in a line can only see that line with no walls-with-deps
// nearby, then they would cause multiple solutions, so all of them need marks.
void find_ambiguous_linear_aligned_row_cells(model::BasicBoard const & board,
                                             AnnotatedMoves &          moves);

void find_ambiguous_linear_aligned_col_cells(model::BasicBoard const & board,
                                             AnnotatedMoves &          moves);

// one-stop shopping for isolated cells, satisfied walls, and walls that can
// be satisfied with the same number of bulbs as open faces. While it does not
// expressly validate the board, it may detect a contradiction and return
// the location of a mark that cannot be illuminated.
OptCoord find_trivial_moves(model::BasicBoard const & board,
                            BoardAnalysis const *     board_analysis,
                            AnnotatedMoves &          moves);

} // namespace solver
