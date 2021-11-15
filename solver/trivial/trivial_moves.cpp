#include "trivial_moves.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include "DebugLog.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"

namespace solver {

using enum model::CellState;
using model::Coord;

using OptMove = std::optional<model::SingleMove>;

bool
is_isolated_empty_cell(Coord coord, model::BasicBoard const & board) {
  bool can_be_lit = false;
  board.visit_rows_cols_outward(coord, [&](Coord, model::CellState cell) {
    can_be_lit |= is_illumable(cell);
    return can_be_lit == false;
  });
  return not can_be_lit;
}

OptMove
find_isolated_empty_cell(model::BasicBoard const & board) {
  OptMove result_move;
  board.visit_empty([&](Coord coord, model::CellState cell) {
    // can it be lit from any direction? If not, we found an isolated empty
    // cell that needs a bulb.
    if (is_isolated_empty_cell(coord, board)) {
      result_move.emplace(model::Action::Add,
                          model::CellState::Empty, // from
                          model::CellState::Bulb,  // to
                          coord);
      return false;
    }
    return true;
  });
  return result_move;
}

OptMove
find_wall_with_deps_equalling_open_faces(model::BasicBoard const & board) {
  OptMove result;
  board.visit_board([&](Coord coord, model::CellState cell) {
    if (int deps = num_wall_deps(cell); deps > 0) {
      int empty_count = 0;
      int bulb_count  = 0;
      board.visit_adjacent(coord, [&](Coord, auto cell) {
        empty_count += cell == Empty;
        bulb_count += cell == Bulb;
      });
      if (empty_count == deps - bulb_count) {
        board.visit_adjacent(coord, [&](Coord coord, auto cell) {
          if (cell == Empty) {
            result.emplace(model::Action::Add,
                           model::CellState::Empty, // from
                           model::CellState::Bulb,  // to
                           coord);
            return false;
          };
          return true;
        });
      }
    }
  });
  return result;
}

OptMove
find_wall_with_satisfied_deps_and_open_faces(model::BasicBoard const & board) {
  OptMove result;
  board.visit_board([&](Coord coord, model::CellState cell) {
    if (int deps = num_wall_deps(cell)) {
      int bulb_count  = 0;
      int empty_count = 0;
      board.visit_adjacent(coord, [&](Coord, auto cell) {
        empty_count += cell == Empty;
        bulb_count += cell == Bulb;
      });
      if (bulb_count == deps && empty_count > 0) {
        board.visit_adjacent(coord, [&](Coord coord, auto cell) {
          if (cell == Empty) {
            result.emplace(model::Action::Add,
                           model::CellState::Empty, // from
                           model::CellState::Mark,  // to
                           coord);
            return false;
          };
          return true;
        });
      }
    }
  });
  return result;
}

void
apply_move(Solution & solution, OptMove opt_move) {
  if (opt_move) {
    LOG_DEBUG("[TRIVIAL] Applying move: {}\n", *opt_move);
    switch (opt_move->to_) {
      case model::CellState::Bulb:
        solution.board_.add_bulb(opt_move->coord_);
        break;

      case model::CellState::Mark:
        solution.board_.add_mark(opt_move->coord_);
        break;

      default:
        break;
    }
  }
}

bool
play_trivial_move(Solution & solution) {
  // trivial moves are played in 1 of 3 situations.  The square is empty
  // and...

  // 1) it cannot be illuminated by any other square, must be a bulb
  // 2) It is adjacent to wall that requires N adjacent bulbs and also has
  // exactly N empty adjacent squares.  Each adjacent square must be a bulb.
  // 3) A wall with N deps has N bulbs next to it, and M open faces.  Each of
  // those M faces must be marked.
  // 4) a marked cell that can only be illuminated by a single other square
  //    on the same row or column.
  auto const & board = solution.board_.board();
  if (auto opt_move = find_isolated_empty_cell(board)) {
    LOG_DEBUG("[TRIVIAL] isolated empty cell: {}\n", *opt_move);
    apply_move(solution, opt_move);
    return true;
  }
  else if (auto opt_move = find_wall_with_deps_equalling_open_faces(board)) {
    LOG_DEBUG("[TRIVIAL] wall with deps==open spaces: {}\n", *opt_move);
    apply_move(solution, opt_move);
    return true;
  }
  else if (auto opt_move =
               find_wall_with_satisfied_deps_and_open_faces(board)) {
    LOG_DEBUG("[TRIVIAL] wall with satisfied deps and open faces: {}\n",
              *opt_move);
    apply_move(solution, opt_move);
    return true;
  }
  return false;
}

} // namespace solver
