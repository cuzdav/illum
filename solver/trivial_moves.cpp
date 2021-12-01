#include "trivial_moves.hpp"
#include "Action.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "Coord.hpp"
#include "Direction.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include "meta.hpp"
#include "utils/DebugLog.hpp"
#include <algorithm>
#include <array>
#include <memory>
#include <optional>

namespace solver {

using enum model::CellState;
using model::CellState;
using model::Coord;
using model::Direction;

namespace {
void
insert_if_unique(AnnotatedMoves & moves, AnnotatedMove const & move) {
  if (std::find_if(
          begin(moves),
          end(moves),
          [where = move.next_move.coord_](AnnotatedMove const & existing) {
            return existing.next_move.coord_ == where;
          }) == end(moves)) {
    moves.push_back(move);
  }
}

void
add_cell(AnnotatedMoves & moves,
         CellState        cell,
         Coord            where,
         DecisionType     why,
         MoveMotive       motive,
         OptCoord         ref_location) {
  insert_if_unique(
      moves,
      AnnotatedMove{
          model::SingleMove{model::Action::Add, CellState::Empty, cell, where},
          why,
          motive,
          ref_location});
}

void
add_bulb(AnnotatedMoves & moves,
         Coord            where,
         DecisionType     why,
         MoveMotive       motive,
         OptCoord         ref_location = std::nullopt) {
  add_cell(moves, CellState::Bulb, where, why, motive, ref_location);
}

void
add_mark(AnnotatedMoves & moves,
         Coord            where,
         DecisionType     why,
         MoveMotive       motive,
         OptCoord         ref_location = std::nullopt) {
  add_cell(moves, CellState::Mark, where, why, motive, ref_location);
}

} // namespace

// This generates the same results as the find_isolated_cells() call, but is
// less efficient because it continues to scan over the same rows and columns
// over and over, where the other one, which is more complicated, tracks where
// it has already scanned for "the impossible" and avoids rescanning it.
OptCoord
find_isolated_cells_old(model::BasicBoard const & board,
                        AnnotatedMoves &          moves) {
  OptCoord unlightable_mark_coord;
  // either a move (where to place a bulb) or an isolated mark, indicating
  // a mark that cannot be illuminated
  board.visit_board([&](Coord coord, CellState cell) {
    if (is_illuminable(cell)) {
      OptCoord empty_neighbor_location;
      int      visible_empty_neighbors = 0;
      board.visit_rows_cols_outward(coord, [&](Coord coord, CellState cell) {
        if (is_empty(cell)) {
          visible_empty_neighbors++;
          empty_neighbor_location = coord;
        }
      });

      if (is_empty(cell) && visible_empty_neighbors == 0) {
        add_bulb(moves,
                 coord,
                 DecisionType::ISOLATED_EMPTY_SQUARE,
                 MoveMotive::FORCED);
      }
      else if (is_mark(cell)) {
        if (visible_empty_neighbors == 1) {
          add_bulb(moves,
                   *empty_neighbor_location,
                   DecisionType::ISOLATED_MARK,
                   MoveMotive::FORCED,
                   coord);
        }
        else if (visible_empty_neighbors == 0) {
          // mark has no visible empty neighbors; cannot be illuminated.
          unlightable_mark_coord = coord;
        }
      }
    }
  });
  return unlightable_mark_coord;
}

// Three cases found:
// 1) an empty cell with no visible empty neighbors. It must contain a bulb
// because it cannot otherwise be illuminated.
// 2) a mark that has exactly one visible empty neighbor. That empty cell must
// contain a bulb because it is the only way to illuminate the mark.
// 3) a mark with zero visible empty neighbors -- board is in an invalid state.
// Returns false if case 3 happened, true otherwise.

// Algorithm has been optimized, which increases complexity, so here's what it
// does:
// 1) scans the board for each cell that is illuminable (empty or a mark),

// 2) then looks in each direction outward, like a castle moves in chess,
// counting the number of empty cells it sees until it runs into a wall (or
// edge of board)

// 3) Since we are looking for isolated empty cells (no visible empty neighbors
// in any direction) or isolated marks (exactly 1 empty neighbor in all
// directions combined), then if we see 2 or more empty cells in either the row
// or column we know that it's impossible to find the scenario we're looking
// for. Therefore, remember how far "down" the column we have seen
// (max_unisolated_row_seen_for_col), or how far "across" this row we have seen,
// because as long as we are still in that column or row, we WONT find a
// solution, so we can skip it.

// It is worth noting that visit_board() is guaranteed to scan the board
// row-wise, starting at the top of board scanning across, then the next row,
// etc. We must store a "how far down" for each column, but only need a single
// "how far across" metric for the current row.

OptCoord
find_isolated_cells(model::BasicBoard const & board, AnnotatedMoves & moves) {

  // track rows/cols already seen that are known to not be isolated.
  // We can skip visiting those.
  OptCoord unlightable_mark_coord;
  int      max_unisolated_row_seen_for_col[model::BasicBoard::MAX_GRID_EDGE]{};
  int      max_unisolated_col_seen_for_row = 0;
  int      cur_row                         = 0;

  board.visit_board([&](Coord coord, CellState cell) {
    // only consider an empty cell or a mark
    if (not model::is_illuminable(cell)) {
      return;
    }
    if (coord.row_ > cur_row) {
      cur_row                         = coord.row_;
      max_unisolated_col_seen_for_row = 0;
    }

    int      max_col         = 0;
    int      max_row         = 0;
    int      empty_row_cells = is_empty(cell);
    int      empty_col_cells = is_empty(cell);
    OptCoord empty_neighbor_location;

    // only scan if we are past the previous range of known row
    // with too many empties to possibly have an isolated cell.
    if (cur_row >= max_unisolated_row_seen_for_col[coord.col_]) {
      board.visit_col_outward(coord, [&](Coord coord, CellState cell) {
        if (coord.row_ > max_row) {
          max_row = coord.row_;
        }
        if (is_empty(cell)) {
          empty_col_cells++;
          empty_neighbor_location = coord;
        }
        if (empty_col_cells >= 2) {
          max_unisolated_row_seen_for_col[coord.col_] = max_row;
        }
      });
    }
    else {
      // skipped col because this is known. Doesn't matter the
      // value, as long as it's >= 2.
      empty_col_cells = 2;
    }

    // only scan if we are past the previous range of known col
    // with too many empties to possibly have an isolated cell.
    if (coord.col_ >= max_unisolated_col_seen_for_row) {
      board.visit_row_outward(coord, [&](Coord coord, CellState cell) {
        if (coord.col_ > max_col) {
          max_col = coord.col_;
        }
        if (is_empty(cell)) {
          empty_row_cells++;
          empty_neighbor_location = coord;
        }
        if (empty_row_cells >= 2) {
          max_unisolated_col_seen_for_row = max_col;
        }
      });
    }
    else {
      // skipped row because this is known to have at least 2
      // empties. Doesn't matter the value, as long as it's
      // >= 2.
      empty_row_cells = 2;
    }

    const int empty_row_col_cells = empty_row_cells + empty_col_cells;

    // an empty at coord is counted twice: once each in row and
    // col, so if the sum is 2, this empty is the only one in
    // this row and col.
    if (is_empty(cell) && empty_row_col_cells == 2) {
      add_bulb(moves,
               coord,
               DecisionType::ISOLATED_EMPTY_SQUARE,
               MoveMotive::FORCED);
      // if we placed a bulb, then update the range of
      // unisolated marks
      max_unisolated_row_seen_for_col[coord.col_] = max_row;
      max_unisolated_col_seen_for_row             = max_col;
    }
    else if (is_mark(cell)) {
      // mark has exactly one empty neighbor; that empty must
      // be a bulb
      if (empty_row_col_cells == 1) {
        add_bulb(moves,
                 *empty_neighbor_location,
                 DecisionType::ISOLATED_MARK,
                 MoveMotive::FORCED,
                 coord);
      }
      else if (empty_row_col_cells == 0) {
        // mark has no visible empty neighbors; cannot be
        // illuminated.
        unlightable_mark_coord = coord;
      }
    }
  });
  return unlightable_mark_coord;
}

void
find_ambiguous_linear_aligned_row_cells(model::BasicBoard const & board,
                                        AnnotatedMoves &          moves) {
  // for any co-linear cells that have no cross-visible illuminable cells
  // Example: The empty cells below the '0' walls are inter-changeable for
  // where the bulb could go, leading to multiple solutions and ambiguity. So
  // they must be marks.``

  //      00.
  //      ...
  //      00.

  // Becomes

  //      00.
  //      XX.
  //      00.

  // and then there is only one place to play to illuminate the marks:
  // Coord(1,2), which also solves it.

  // This algorithm tests every separate horizontal row section in a single
  // pass of the board.
  int row = 0;
  int col = -1;
  while (row < board.height()) {
    int      count = 0;
    OptCoord prev;
    board.visit_row_right_of(Coord{row, col}, [&](Coord coord, CellState cell) {
      col = coord.col_;
      if (is_empty(cell)) {
        bool constrained = false;
        board.visit_adjacent(coord, [&](Coord, CellState cell) {
          constrained |= model::is_wall_with_deps(cell);
        });
        board.visit_perpendicular(
            coord, Direction::Right, [&](Coord, CellState cell) {
              constrained |= model::is_illuminable(cell);
              return constrained ? model::STOP_VISITING : model::KEEP_VISITING;
            });
        if (not constrained) {
          if (prev) {
            add_mark(moves,
                     *prev,
                     DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                     MoveMotive::FOLLOWUP);
          }
          ++count;
          prev = coord;
        }
      }
    });
    if (count > 1) {
      add_mark(moves,
               *prev,
               DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
               MoveMotive::FOLLOWUP);
      return;
    }
    if (col + 1 >= board.width()) {
      col = -1;
      ++row;
    }
  }
}

void
find_ambiguous_linear_aligned_col_cells(model::BasicBoard const & board,
                                        AnnotatedMoves &          moves) {
  // vertical equivalent of the by-row version.  See its commentary.

  int row = -1;
  int col = 0;
  while (col < board.width()) {
    int      count = 0;
    OptCoord prev;
    board.visit_col_below(Coord{row, col}, [&](Coord coord, CellState cell) {
      row = coord.row_;
      if (is_empty(cell)) {
        bool constrained = false;
        board.visit_adjacent(coord, [&](Coord, CellState cell) {
          constrained |= model::is_wall_with_deps(cell);
        });
        if (not constrained) {
          board.visit_perpendicular(
              coord, Direction::Down, [&](Coord, CellState cell) {
                constrained |= model::is_illuminable(cell);
                return constrained ? model::STOP_VISITING
                                   : model::KEEP_VISITING;
              });
        }
        if (not constrained) {
          if (prev) {
            add_mark(moves,
                     *prev,
                     DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                     MoveMotive::FOLLOWUP);
          }
          ++count;
          prev = coord;
        }
      }
    });
    if (count > 1) {
      add_mark(moves,
               *prev,
               DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
               MoveMotive::FOLLOWUP);
      return;
    }
    if (row + 1 >= board.height()) {
      row = -1;
      ++col;
    }
  }
}

std::unique_ptr<BoardAnalysis>
create_board_analysis(model::BasicBoard const & board) {
  std::unique_ptr result = std::make_unique<BoardAnalysis>();
  result->walls_with_deps.reserve(16);
  board.visit_board([&](Coord wall_coord, CellState cell) {
    if (int deps = num_wall_deps(cell); deps > 0) {
      result->walls_with_deps.push_back(wall_coord);
    }
  });
  return result;
}

// for performance, merges 2 algos into 1:
// find_walls_with_deps_equal_open_faces()
// find_satisfied_walls_having_open_faces() same metadata and structure,
// cells of interest. Only difference between the 2 algos is the inner if.
void
find_around_walls_with_deps(model::BasicBoard const & board,
                            BoardAnalysis const *     board_analysis,
                            AnnotatedMoves &          moves) {
  for (Coord wall_coord : board_analysis->walls_with_deps) {
    CellState cell        = board.get_cell(wall_coord);
    int       empty_count = 0;
    int       bulb_count  = 0;
    int       deps        = num_wall_deps(cell);
    board.visit_adjacent(wall_coord, [&](Coord coord, auto cell) {
      empty_count += cell == Empty;
      bulb_count += cell == Bulb;
    });

    // all empty faces around wall must be bulbs
    if (empty_count > 0 && (empty_count == deps - bulb_count)) {
      board.visit_adjacent(wall_coord, [&](Coord adj_coord, auto cell) {
        if (is_empty(cell)) {
          add_bulb(moves,
                   adj_coord,
                   DecisionType::WALL_DEPS_EQUAL_OPEN_FACES,
                   MoveMotive::FORCED,
                   wall_coord);
        }
      });
    }
    // all empty faces around wall must be marks (wall satisfied)
    if (empty_count > 0 && bulb_count == deps) {
      board.visit_adjacent(wall_coord, [&](Coord adj_coord, auto cell) {
        if (is_empty(cell)) {
          add_mark(moves,
                   adj_coord,
                   DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES,
                   MoveMotive::FORCED,
                   wall_coord);
        }
      });
    }
  }
}

OptCoord
find_trivial_moves(model::BasicBoard const & board,
                   BoardAnalysis const *     board_analysis,
                   AnnotatedMoves &          moves) {
  find_around_walls_with_deps(board, board_analysis, moves);
  if (moves.empty()) {
    find_ambiguous_linear_aligned_row_cells(board, moves);
  }
  if (moves.empty()) {
    find_ambiguous_linear_aligned_col_cells(board, moves);
  }
  return find_isolated_cells(board, moves);
}

} // namespace solver
