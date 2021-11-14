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
using model::CellState;
using model::Coord;

constexpr int MAX_SOLVE_STEPS = 10000;

// Speculation involves "trying" to place a bulb (or mark) in each empty cell,
// and playing it out until:
// 1) an unambiguous full solution is found
// 2) an invalid board is encountered

// After finding a solution, speculation must continue with other moves to
// ensure all other options lead to an invalid board. If multiple solutions are
// found, then at least one move in the path to where we are was in error.
//
// There may be room for optimization here to reduce speculating the same moves.
// However, since the moment we encounter an invalid board we stop this path,
// and we only begin speculation when there are no trivial moves remaining,
// we're essentially speculating against empty and unsatisfied walls only, and
// all solutions found will not interact with any existing bulbs or satisfied
// walls.
bool
speculate_playing_bulbs(Solution & solution) {
  solution.board_.board().visit_board([&](Coord coord, CellState cell) {

  });

  return false;
}

bool
speculate_playing_marks(Solution & solution) {
  return false;
}

void
play_move(Solution & solution) {
  ++solution.step_count_;
  if (play_trivial_move(solution)) {
    return;
  }
  else if (speculate_playing_bulbs(solution)) {
    return;
  }
  else if (speculate_playing_marks(solution))
    solution.status_ = SolutionStatus::FailedFindingMove;
}

void
play_single_move(Solution & solution) {
  if (solution.board_.is_solved()) {
    solution.status_ = SolutionStatus::Solved;
  }
  else {
    play_move(solution);
  }
}

Solution
solve(model::BasicBoard const & board) {
  Solution solution(board);
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
