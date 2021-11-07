#include "Solver.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include "trivial/trivial_moves.hpp"
#include <optional>
#include <utility>

namespace solver {
using enum model::CellState;
using model::Coord;

constexpr int MAX_SOLVE_STEPS = 10000;

void
play_move(Solution & solution) {
  ++solution.step_count_;
  if (play_trivial_move(solution)) {
    return;
  }
  solution.status_ = SolutionStatus::FailedFindingMove;
}

// brute force
bool
check_solved(model::BasicBoard const & board) {
  using enum model::CellState;
  bool solved = true;
  board.visit_board([&](Coord coord, model::CellState cell) {
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
      board.visit_adjacent(coord, [&](Coord, model::CellState adj_cell) {
        requires_adjacent -= adj_cell == Bulb;
      });
      if (requires_adjacent != 0) {
        solved = false;
      }
      break;

    case Bulb: {
      bool sees_other_bulb = false;

      // return true/false to keep visiting or to stop
      auto can_see_bulb = [&](Coord coord, auto cell) {
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

      board.visit_row_left_of(coord, can_see_bulb);
      not sees_other_bulb && board.visit_row_left_of(coord, can_see_bulb);
      not sees_other_bulb && board.visit_row_right_of(coord, can_see_bulb);
      not sees_other_bulb && board.visit_col_above(coord, can_see_bulb);
      not sees_other_bulb && board.visit_col_below(coord, can_see_bulb);

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

void
play_single_move(Solution & solution) {
  if (check_solved(solution.model_.get_underlying_board())) {
    solution.status_ = SolutionStatus::Solved;
  }
  else {
    play_move(solution);
  }
}

Solution
solve(model::BasicBoard const & board) {
  Solution solution;
  solution.model_.reset_game(board);
  solution.status_ = SolutionStatus::Progressing;

  do {
    play_single_move(solution);
  } while (solution.status_ == SolutionStatus::Progressing &&
           solution.step_count_ < MAX_SOLVE_STEPS);

  if (solution.status_ == SolutionStatus::Progressing) {
    solution.status_ = SolutionStatus::Terminated;
  }

  return solution;
}

} // namespace solver
