#include "Solver.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "DebugLog.hpp"
#include "Direction.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include "scope.hpp"
#include "trivial/trivial_moves.hpp"
#include <optional>
#include <utility>

namespace solver {
using enum model::CellState;
using model::CellState;
using model::Coord;

constexpr int MAX_SOLVE_STEPS = 10000;

void find_solution(Solution & solution);

// Speculation involves "trying" to place a bulb (or mark) in each empty cell,
// and playing out the trivial/forced moves as a result to see if it causes a
// contradiction. If it does not cause trivial/forced moves, or they play out
// without contradiction until the board is stable again, it is inconclusive and
// we must backtrack.

// If it solves the board, we only care if:
// 1) they are only forced moves from here out, and
// 2) we continue backtracking, to see if another move also solves it.  If so,
// that ambiguity is a contradiction and we failed (earlier in the path to get
// here)
bool
speculate_playing_bulbs(Solution & solution) {
  LOG_DEBUG("speculate playing_bulbs: step_count: {}, status: {}\n",
            solution.step_count_,
            to_string(solution.status_));

  Solution::OptBoard one_solution;
  bool               played_move = false;
  auto &             basic_board = solution.board_.board();
  basic_board.visit_empty([&](Coord coord, CellState) {
    // Speculative move
    solution.step_count_++;
    solution.board_.clone_position();

    LOG_DEBUG("[SPECULATE-BULB] Playing ({},{})\n", coord.row_, coord.col_);
    solution.board_.add_bulb(coord);
    play_trivial_move(solution);

    if (solution.board_.has_error()) {
      // This is actually success - we found a contradiction with this move
      LOG_DEBUG("[SPECULATE-BULB] Contradiction, so  ({},{}) must be a mark\n",
                coord.row_,
                coord.col_);
      solution.board_.pop();
      solution.board_.add_mark(coord);
      played_move = true;
      return false; // stop visiting
    }
    else if (solution.board_.is_solved()) {
      if (not one_solution.has_value()) {
        solution.board_.pop();
        one_solution.emplace(solution.board_.board());
        // keep visiting to see if there are no more solutions
        // if we finish and this is the only one, that's success.
        return true;
      }
      else if (solution.board_.board() != *one_solution) {
        // multiple different solutions?  Uh oh.
        solution.board_.pop();
        solution.status_ = SolutionStatus::Ambiguous;
        return false; // stop visiting
      }
    }
    solution.board_.pop();
    return true; // keep visiting
  });
  LOG_DEBUG("Done visiting empty spaces (speculate) steps={} {}\n",
            solution.step_count_,
            to_string(solution.status_));
  if (one_solution.has_value()) {
    solution.status_         = SolutionStatus::Solved;
    solution.known_solution_ = *one_solution;
    return true;
  }
  return played_move;
}

bool
speculate_playing_marks(Solution & solution) {
  LOG_DEBUG("speculate marks: step_count: {}, status: {}\n",
            solution.step_count_,
            to_string(solution.status_));
  return false;
}

bool
play_move(Solution & solution) {
  if (play_trivial_move(solution)) {
    return true;
  }

  LOG_DEBUG("PLAY MOVE - returned from play trivial: is speculating? {}\n",
            solution.speculating_);

  // no speculation within speculation!
  if (solution.speculating_) {
    return false;
  }

  LOG_DEBUG("Begin speculation\n");
  solution.speculating_ = true;
  pc::scoped_exit const nothrow{
      [&]() noexcept { solution.speculating_ = false; }};

  if (speculate_playing_bulbs(solution)) {
    return true;
  }
  if (speculate_playing_marks(solution)) {
    return true;
  }
  solution.status_ = SolutionStatus::FailedFindingMove;
  return false;
}

bool
play_single_move(Solution & solution) {
  if (solution.board_.is_solved()) {
    if (not solution.speculating_) {
      LOG_DEBUG("play_single_move: Solved! steps={}\n", solution.step_count_);
      solution.status_ = SolutionStatus::Solved;
      return true;
    }
  }
  else if (solution.board_.has_error()) {
    if (not solution.speculating_) {
      LOG_DEBUG("play_single_move: ERROR! steps={}\n", solution.step_count_);
      solution.status_ = SolutionStatus::Impossible;
      return true;
    }
  }
  else {
    return play_move(solution);
  }
  return false;
}

void
find_solution(Solution & solution) {
  do {
    // LOG_DEBUG("Top of find_solution loop.  {} steps={}\n",
    //           to_string(solution.status_),
    //           solution.step_count_);
    solution.step_count_++;
    if (not play_single_move(solution)) {
      break;
    }
  } while (solution.status_ == SolutionStatus::Progressing &&
           solution.step_count_ < MAX_SOLVE_STEPS);
}

Solution
solve(model::BasicBoard const & board) {
  Solution solution(board);
  solution.status_ = SolutionStatus::Progressing;

  find_solution(solution);

  if (solution.status_ == SolutionStatus::Progressing) {
    solution.status_ = SolutionStatus::Terminated;
  }

  return solution;
}

} // namespace solver
