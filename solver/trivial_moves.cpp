#include "trivial_moves.hpp"
#include "Action.hpp"
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "Coord.hpp"
#include "Direction.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include "meta.hpp"
#include "utils/DebugLog.hpp"
#include <algorithm>
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

// Three cases found:
// 1) an empty cell with no visible empty neighbors. It must contain a bulb
// because it cannot otherwise be illuminated.
// 2) a mark that has exactly one visible empty neighbor. That empty cell must
// contain a bulb because it is the only way to illuminate the mark.
// 3) a mark with zero visible empty neighbors -- board is in an invalid state.
// Returns false if case 3 happened, true otherwise.
OptCoord
find_isolated_cells(model::BasicBoard const & board, AnnotatedMoves & moves) {
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

void
find_ambiguous_linear_aligned_row_cells(model::BasicBoard const & board,
                                        AnnotatedMoves &          moves) {
  // for any co-linear cells that have no cross-visible illuminable cells
  // Example: The empty cells below the '0' walls are inter-changeable for where
  // the bulb could go, leading to multiple solutions and ambiguity. So they
  // must be marks.``

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
        board.visit_perpendicular(
            coord, Direction::Down, [&](Coord, CellState cell) {
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
    if (row + 1 >= board.height()) {
      row = -1;
      ++col;
    }
  }
}

void
find_walls_with_deps_equal_open_faces(model::BasicBoard const & board,
                                      AnnotatedMoves &          moves) {
  board.visit_board([&](Coord wall_coord, CellState cell) {
    if (int deps = num_wall_deps(cell); deps > 0) {
      int empty_count = 0;
      int bulb_count  = 0;
      board.visit_adjacent(wall_coord, [&](Coord, auto cell) {
        empty_count += cell == Empty;
        bulb_count += cell == Bulb;
      });
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
    }
  });
}

void
find_satisfied_walls_having_open_faces(model::BasicBoard const & board,
                                       AnnotatedMoves &          moves) {
  board.visit_board([&](Coord coord, CellState cell) {
    if (int deps = num_wall_deps(cell)) {
      int bulb_count  = 0;
      int empty_count = 0;
      board.visit_adjacent(coord, [&](Coord, auto cell) {
        empty_count += cell == Empty;
        bulb_count += cell == Bulb;
      });
      if (bulb_count == deps && empty_count > 0) {
        board.visit_adjacent(coord, [&](Coord adj_coord, auto cell) {
          if (is_empty(cell)) {
            add_mark(moves,
                     adj_coord,
                     DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES,
                     MoveMotive::FORCED,
                     coord);
          }
        });
      }
    }
    return model::KEEP_VISITING;
  });
}

OptCoord
find_trivial_moves(model::BasicBoard const & board, AnnotatedMoves & moves) {
  find_satisfied_walls_having_open_faces(board, moves);
  find_walls_with_deps_equal_open_faces(board, moves);
  find_ambiguous_linear_aligned_row_cells(board, moves);
  find_ambiguous_linear_aligned_col_cells(board, moves);
  return find_isolated_cells(board, moves);
}

} // namespace solver
