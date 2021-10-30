#include "Solver.hpp"
#include "Solution.hpp"
#include <utility>

namespace solver {

constexpr int MAX_SOLVE_STEPS = 10000;

bool
play_trivial_move(Solution & solution) {
  return false;
}

void
play_move(Solution & solution) {
  ++solution.step_count_;
  if (play_trivial_move(solution)) {
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
