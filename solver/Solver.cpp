#include "Solver.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include <optional>
#include <utility>

namespace solver {

constexpr int MAX_SOLVE_STEPS = 10000;

bool
visit_row_from(model::BasicBoard const & board, int row, int col, int dx) {}

std::optional<model::SingleMove>
find_isolated_empty_cell(model::BasicBoard const & board) {

  board.visit_board([&](int row, int col, model::CellState state) {

  });
  return {};
}

void
play_bulb(model::SingleMove const & move, Solution & solution) {
  // TODO
}

bool
play_trivial_bulb(Solution & solution) {
  // trivial bulbs are played in 1 of 2 situations.  The square is empty and...

  // 1) it cannot be illuminated by any other square
  // 2) It is adjacent to wall that requires N adjacent bulbs and also has
  // exactly N empty adjacent squares.

  if (auto opt_move =
          find_isolated_empty_cell(solution.model_.get_underlying_board())) {
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

Solution
solve(model::BasicBoard const & board) {
  Solution solution;
  solution.model_.reset_game(board);
  solution.status_ = SolutionStatus::Progressing;

  do {
    play_move(solution);
  } while (solution.status_ == SolutionStatus::Progressing &&
           solution.step_count_ < MAX_SOLVE_STEPS);

  if (solution.status_ == SolutionStatus::Progressing) {
    solution.status_ = SolutionStatus::Terminated;
  }

  return solution;
}

} // namespace solver
