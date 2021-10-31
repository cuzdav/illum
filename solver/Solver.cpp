#include "Solver.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include <optional>
#include <utility>

namespace solver {
using enum model::CellState;

constexpr int MAX_SOLVE_STEPS = 10000;

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

void
play_bulb(model::SingleMove const & move, Solution & solution) {
  solution.model_.add(model::CellState::Bulb, move.row_, move.col_);
}

std::optional<model::SingleMove>
find_wall_with_deps_equalling_open_faces(model::BasicBoard const & board) {
  std::optional<model::SingleMove> result;
  board.visit_board([&](int row, int col, model::CellState cell) {
    if (int deps = num_wall_deps(cell)) {
      int empty_count = 0;
      board.visit_adjacent(
          row, col, [&](int, int, auto cell) { empty_count += cell == Empty; });
      if (empty_count == deps) {
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

bool
play_trivial_bulb(Solution & solution) {
  // trivial bulbs are played in 1 of 2 situations.  The square is empty
  // and...

  // 1) it cannot be illuminated by any other square
  // 2) It is adjacent to wall that requires N adjacent bulbs and also has
  // exactly N empty adjacent squares.
  auto & board = solution.model_.get_underlying_board();
  if (auto opt_move = find_isolated_empty_cell(board)) {
    play_bulb(*opt_move, solution);
    return true;
  }
  else if (auto opt_move = find_wall_with_deps_equalling_open_faces(board)) {
    play_bulb(*opt_move, solution);
    return true;
  }

  return false;
}

void
play_move(Solution & solution) {
  ++solution.step_count_;
  if (play_trivial_bulb(solution)) {
    return;
  }

  solution.status_ = SolutionStatus::FailedFindingMove;
}

bool
check_solved(model::BasicBoard const & board) {
  using enum model::CellState;
  bool solved = true;
  board.visit_board([&](int row, int col, model::CellState cell) {
    int requires_adjacent = 0;

    switch (cell) {
    case Wall0:
    case Illum: break;

    case Mark: ++requires_adjacent; [[fallthrough]];
    case Empty: solved = false; break;

    case Wall4: ++requires_adjacent; [[fallthrough]];
    case Wall3: ++requires_adjacent; [[fallthrough]];
    case Wall2: ++requires_adjacent; [[fallthrough]];
    case Wall1:
      ++requires_adjacent;
      board.visit_adjacent(row, col, [&](int, int, model::CellState adj_cell) {
        requires_adjacent -= adj_cell == Bulb;
      });
      if (requires_adjacent != 0) {
        solved = false;
      }
      break;

    case Bulb: {
      bool sees_other_bulb = false;
      auto can_see_bulb    = [&](int r, int c, auto cell) {
        if ((cell & model::any_wall) == cell) {
          // done scanning in this direction
          return false;
        }
        else if (cell == Bulb) {
          sees_other_bulb = true;
          return false;
        }
        return true;
      };

      board.visit_row_left_of(row, col, can_see_bulb);
      not sees_other_bulb && board.visit_row_left_of(row, col, can_see_bulb);
      not sees_other_bulb && board.visit_row_right_of(row, col, can_see_bulb);
      not sees_other_bulb && board.visit_col_above(row, col, can_see_bulb);
      not sees_other_bulb && board.visit_col_below(row, col, can_see_bulb);

      if (sees_other_bulb) {
        solved = false;
        return false;
      }
      break;
    }
    }
    return true;
  });
  return solved;
}

Solution
solve(model::BasicBoard const & board) {
  Solution solution;
  solution.model_.reset_game(board);
  solution.status_ = SolutionStatus::Progressing;

  do {
    if (check_solved(solution.model_.get_underlying_board())) {
      solution.status_ = SolutionStatus::Solved;
      break;
    }
    play_move(solution);
  } while (solution.status_ == SolutionStatus::Progressing &&
           solution.step_count_ < MAX_SOLVE_STEPS);

  if (solution.status_ == SolutionStatus::Progressing) {
    solution.status_ = SolutionStatus::Terminated;
  }

  return solution;
}

} // namespace solver
