#include "trivial_moves.hpp"
#include "Action.hpp"
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include "meta.hpp"
#include "utils/DebugLog.hpp"
#include <algorithm>
#include <optional>

namespace solver {

using enum model::CellState;
using model::Coord;

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
} // namespace

// Three cases found:
// 1) an empty cell with no visible empty neighbors. It must contain a bulb
// because it cannot otherwise be illuminated.
// 2) a mark that has exactly one visible empty neighbor. That empty cell must
// contain a bulb because it is the only way to illuminate the mark.
// 3) a mark with zero visible empty neighbors -- board is in an invalid state.
// Returns false if case 3 happened, true otherwise.
bool
find_isolated_cells(model::BasicBoard const & board, AnnotatedMoves & moves) {
  bool board_is_valid = true;

  // either a move (where to place a bulb) or an isolated mark, indicating
  // a mark that cannot be illuminated
  board.visit_board([&](Coord coord, model::CellState cell) {
    if (is_illumable(cell)) {
      OptCoord empty_neighbor_location;
      int      visible_empty_neighbors = 0;
      board.visit_rows_cols_outward(coord,
                                    [&](Coord coord, model::CellState cell) {
                                      if (is_empty(cell)) {
                                        visible_empty_neighbors++;
                                        empty_neighbor_location = coord;
                                      }
                                    });

      if (is_empty(cell) && visible_empty_neighbors == 0) {
        insert_if_unique(
            moves,
            AnnotatedMove{model::SingleMove{model::Action::Add,
                                            model::CellState::Empty, // from
                                            model::CellState::Bulb,  // to
                                            coord},
                          DecisionType::ISOLATED_EMPTY_SQUARE,
                          MoveMotive::FORCED,
                          coord});
      }
      else if (is_mark(cell)) {
        if (visible_empty_neighbors == 1) {
          insert_if_unique(
              moves,
              AnnotatedMove{model::SingleMove{model::Action::Add,
                                              model::CellState::Empty, // from
                                              model::CellState::Bulb,  // to
                                              *empty_neighbor_location},
                            DecisionType::ISOLATED_MARK,
                            MoveMotive::FORCED,
                            coord});
        }
        else if (visible_empty_neighbors == 0) {
          // mark has no visible neighbors.  Board
          board_is_valid = false;
        }
      }
    }
  });
  return board_is_valid;
}

void
find_walls_with_deps_equal_open_faces(model::BasicBoard const & board,
                                      AnnotatedMoves &          moves) {
  board.visit_board([&](Coord wall_coord, model::CellState cell) {
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
            insert_if_unique(
                moves,
                AnnotatedMove{model::SingleMove{model::Action::Add,
                                                model::CellState::Empty, // from
                                                model::CellState::Bulb,  // to
                                                adj_coord},
                              DecisionType::WALL_DEPS_EQUAL_OPEN_FACES,
                              MoveMotive::FORCED,
                              wall_coord});
          }
        });
      }
    }
  });
}

void
find_satisfied_walls_having_open_faces(model::BasicBoard const & board,
                                       AnnotatedMoves &          moves) {
  board.visit_board([&](Coord coord, model::CellState cell) {
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
            insert_if_unique(
                moves,
                AnnotatedMove{model::SingleMove{model::Action::Add,
                                                model::CellState::Empty, // from
                                                model::CellState::Mark,  // to
                                                adj_coord},
                              DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES,
                              MoveMotive::FORCED,
                              coord});
          }
        });
      }
    }
    return model::KEEP_VISITING;
  });
}

bool
find_trivial_moves(model::BasicBoard const & board, AnnotatedMoves & moves) {
  find_satisfied_walls_having_open_faces(board, moves);
  find_walls_with_deps_equal_open_faces(board, moves);

  bool is_valid = find_isolated_cells(board, moves);

  std::sort(begin(moves), end(moves));
  moves.erase(std::unique(begin(moves), end(moves)), end(moves));

  return is_valid;
}

} // namespace solver
