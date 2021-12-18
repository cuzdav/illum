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
          model::SingleMove{model::Action::ADD, CellState::EMPTY, cell, where},
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
  add_cell(moves, CellState::BULB, where, why, motive, ref_location);
}

void
add_mark(AnnotatedMoves & moves,
         Coord            where,
         DecisionType     why,
         MoveMotive       motive,
         OptCoord         ref_location = std::nullopt) {
  add_cell(moves, CellState::MARK, where, why, motive, ref_location);
}

int
get_col_span_empty_count(Coord                              coord,
                         model::BasicBoard const &          board,
                         std::vector<CellSpanCount> const & col_span_cache) {
  auto col_span_iter = std::lower_bound(
      begin(col_span_cache),
      end(col_span_cache),
      coord,
      [&](CellSpanCount const & cell_span_count, Coord coord) {
        return cell_span_count.span_start_coord.col_ < coord.col_;
      });

  int count = -1;
  while (col_span_iter != end(col_span_cache) &&
         col_span_iter->span_start_coord.col_ <= coord.col_ &&
         col_span_iter->span_start_coord.row_ <= coord.row_) {
    assert(col_span_iter->span_start_coord.col_ == coord.col_);
    count = col_span_iter->count;
    ++col_span_iter;
  }

  if (count != -1) {
    return count;
  }

  throw std::runtime_error(fmt::format(
      "Should not be possible; no column span associated with {}", coord));
}

std::vector<CellSpanCount> &
init_row_span_cache(model::BasicBoard const & board,
                    BoardAnalysis *           board_analysis) {
  // count all of the row spans... that is, for each contiguous range of
  // (empty|mark) cells, count the empties, and store them in the sorted
  // vector keyed by the coordinate of the FIRST cell in the span. Whenever
  // any other type of cell is seen, it ends that segment.
  auto & row_span_cache = board_analysis->row_span_cache;
  row_span_cache.clear();
  int flat_idx = 0;
  for (int row = 0; row < board.height(); ++row) {
    bool in_span = false;
    for (int col = 0; col < board.width(); ++col, flat_idx++) {
      CellState cell = board.get_cell_flat_unchecked(flat_idx);
      if (model::is_translucent(cell)) {
        if (not in_span) {
          in_span = true;
          row_span_cache.push_back(CellSpanCount{Coord{row, col}, 0});
        }
        if (model::is_empty(cell)) {
          row_span_cache.back().count++;
        }
      }
      else {
        in_span = false;
      }
    }
  }
  return row_span_cache;
}

std::vector<CellSpanCount> &
init_col_span_cache(model::BasicBoard const & board,
                    BoardAnalysis *           board_analysis) {

  auto & col_span_cache = board_analysis->col_span_cache;
  col_span_cache.clear();
  for (int col = 0; col < board.width(); ++col) {
    bool in_span  = false;
    int  flat_idx = col;
    for (int row = 0; row < board.height(); ++row, flat_idx += board.width()) {
      CellState cell = board.get_cell_flat_unchecked(flat_idx);
      if (model::is_translucent(cell)) {
        if (not in_span) {
          in_span = true;
          col_span_cache.push_back(CellSpanCount{Coord{row, col}, 0});
        }
        if (model::is_empty(cell)) {
          col_span_cache.back().count++;
        }
      }
      else {
        in_span = false;
      }
    }
  }
  return col_span_cache;
}

} // namespace

// Three cases found:
// 1) an empty cell with no visible empty neighbors. It must contain a bulb
// because it cannot otherwise be illuminated.
// 2) a mark that has exactly one visible empty neighbor. That empty cell must
// contain a bulb because it is the only way to illuminate the mark.
// 3) a mark with zero visible empty neighbors -- board is in an invalid
// state. Returns false if case 3 happened, true otherwise.
OptCoord
find_isolated_cells(model::BasicBoard const & board,
                    BoardAnalysis *           board_analysis,
                    AnnotatedMoves &          moves) {

  auto & row_span_cache = init_row_span_cache(board, board_analysis);
  auto & col_span_cache = init_col_span_cache(board, board_analysis);

  // now walk the board, and find any isolated empty cells, or marks, using
  // the caches we just populated. Just find the row span and column span it
  // is in, and sum them up to know the number of empties in all its
  // directions, and account for what the cell is itself.
  OptCoord unlightable_mark_coord;
  for (auto & cur_row_span : row_span_cache) {
    board.visit_row_right_of(
        cur_row_span.span_start_coord,
        [&](Coord coord, CellState cell) {
          if (not is_illuminable(cell)) {
            return;
          }
          const int row_col_empty_count =
              cur_row_span.count +
              get_col_span_empty_count(coord, board, col_span_cache);

          if (is_empty(cell) && row_col_empty_count == 2) {
            add_bulb(moves,
                     coord,
                     DecisionType::ISOLATED_EMPTY_SQUARE,
                     MoveMotive::FORCED);
          }
          else if (is_mark(cell)) {
            if (row_col_empty_count == 1) {
              // this is an isolated mark but we don't know where
              // its empty cell is. Find it.
              board.visit_rows_cols_outward(
                  coord, [&](auto adj_coord, auto adj_cell) {
                    if (is_empty(adj_cell)) {
                      add_bulb(moves,
                               adj_coord,
                               DecisionType::ISOLATED_MARK,
                               MoveMotive::FORCED,
                               coord);
                      return model::STOP_VISITING;
                    }
                    return model::KEEP_VISITING;
                  });
            }
            else if (row_col_empty_count == 0) {
              std::cout << "Unlightable mark at : " << coord << "\n";
              unlightable_mark_coord = coord;
            }
          }
        },
        (model::BasicBoard::VisitPolicy::VISIT_START_COORD |
         model::BasicBoard::VisitPolicy::SKIP_TERMINATING_WALL));
  }
  return unlightable_mark_coord;
}

void
find_ambiguous_linear_aligned_row_cells(model::BasicBoard const & board,
                                        AnnotatedMoves &          moves) {
  // for any co-linear cells that have no cross-visible illuminable cells
  // Example: The empty cells below the '0' walls are inter-changeable for
  // where the bulb could go, leading to multiple solutions and ambiguity.
  // So they must be marks.``

  //      00.
  //      ...
  //      00.

  // Becomes

  //      00.
  //      XX.
  //      00.

  // and then there is only one place to play to illuminate the marks:
  // Coord(1,2), which also solves it.

  // This algorithm tests every separate horizontal row section in a
  // single pass of the board.
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
            coord, Direction::RIGHT, [&](Coord, CellState cell) {
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
              coord, Direction::DOWN, [&](Coord, CellState cell) {
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
  std::vector<model::Coord> walls_with_deps;
  walls_with_deps.reserve(16);
  board.visit_board([&](Coord wall_coord, CellState cell) {
    if (int deps = num_wall_deps(cell); deps > 0) {
      walls_with_deps.push_back(wall_coord);
    }
  });

  return std::make_unique<BoardAnalysis>(walls_with_deps);
}

// for performance, merges 2 algos into 1:
// find_walls_with_deps_equal_open_faces()
// find_satisfied_walls_having_open_faces() same metadata and structure,
// cells of interest. Only difference between the 2 algos is the inner if.
void
find_around_walls_with_deps(model::BasicBoard const & board,
                            BoardAnalysis *           board_analysis,
                            AnnotatedMoves &          moves) {
  for (Coord wall_coord : board_analysis->walls_with_deps) {
    CellState cell        = board.get_cell(wall_coord);
    int       empty_count = 0;
    int       bulb_count  = 0;
    int       deps        = num_wall_deps(cell);
    board.visit_adjacent(wall_coord, [&](Coord coord, auto cell) {
      empty_count += cell == EMPTY;
      bulb_count += cell == BULB;
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
                   BoardAnalysis *           board_analysis,
                   AnnotatedMoves &          moves) {
  find_around_walls_with_deps(board, board_analysis, moves);
  if (moves.empty()) {
    find_ambiguous_linear_aligned_row_cells(board, moves);
  }
  if (moves.empty()) {
    find_ambiguous_linear_aligned_col_cells(board, moves);
  }
  return find_isolated_cells(board, board_analysis, moves);
}

} // namespace solver
