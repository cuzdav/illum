#include "trivial_moves.hpp"
#include "CellState.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"

namespace solver {

using enum model::CellState;

bool
is_isolated_empty_cell(int row, int col, model::BasicBoard const & board) {
  bool can_be_lit = false;
  board.visit_rows_cols_outward(
      row, col, [&](int r, int c, model::CellState cell) {
        if ((cell & model::any_wall) == cell) {
          return false;
        }
        can_be_lit |= (cell & (Empty | Bulb)) == cell;
        return not can_be_lit;
      });
  return not can_be_lit;
}

std::optional<model::SingleMove>
find_isolated_empty_cell(model::BasicBoard const & board) {
  std::optional<model::SingleMove> result;
  board.visit_board([&](int row, int col, model::CellState cell) {
    if (cell == Empty) {
      // can it be lit from any direction? If not, we found an isolated empty
      // cell that needs a bulb.
      if (is_isolated_empty_cell(row, col, board)) {
        result.emplace(model::Action::Add, model::CellState::Bulb, row, col);
        return false;
      }
    }
    return true;
  });
  return result;
}

std::optional<model::SingleMove>
find_wall_with_deps_equalling_open_faces(model::BasicBoard const & board) {
  std::optional<model::SingleMove> result;
  board.visit_board([&](int row, int col, model::CellState cell) {
    if (int deps = num_wall_deps(cell)) {
      int empty_count = 0;
      int bulb_count  = 0;
      board.visit_adjacent(row, col, [&](int, int, auto cell) {
        empty_count += cell == Empty;
        bulb_count += cell == Bulb;
      });
      if (empty_count == deps - bulb_count) {
        board.visit_adjacent(row, col, [&](int, int, auto cell) {
          if (cell == Empty) {
            result.emplace(
                model::Action::Add, model::CellState::Bulb, row, col);
            return false;
          };
          return true;
        });
      }
    }
  });
  return result;
}

std::optional<model::SingleMove>
find_wall_with_satisfied_deps_and_open_faces(model::BasicBoard const & board) {
  std::optional<model::SingleMove> result;
  board.visit_board([&](int row, int col, model::CellState cell) {
    if (int deps = num_wall_deps(cell)) {
      int bulb_count  = 0;
      int empty_count = 0;
      board.visit_adjacent(row, col, [&](int, int, auto cell) {
        empty_count += cell == Empty;
        bulb_count += cell == Bulb;
      });
      if (bulb_count == deps && empty_count > 0) {
        board.visit_adjacent(row, col, [&](int row, int col, auto cell) {
          if (cell == Empty) {
            result.emplace(
                model::Action::Add, model::CellState::Mark, row, col);
            return false;
          };
          return true;
        });
      }
    }
  });
  return result;
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
  auto & board = solution.model_.get_underlying_board();
  if (auto opt_move = find_isolated_empty_cell(board)) {
    solution.model_.apply(*opt_move);
    return true;
  }
  else if (auto opt_move = find_wall_with_deps_equalling_open_faces(board)) {
    solution.model_.apply(*opt_move);
    return true;
  }
  else if (auto opt_move =
               find_wall_with_satisfied_deps_and_open_faces(board)) {
    solution.model_.apply(*opt_move);
    return true;
  }
  return false;
}

} // namespace solver
