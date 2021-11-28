#include "Solver.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "DecisionType.hpp"
#include "Direction.hpp"
#include "PositionBoard.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include "SpeculationContext.hpp"
#include "scope.hpp"
#include "trivial_moves.hpp"
#include "utils/DebugLog.hpp"
#include <algorithm>
#include <fmt/core.h>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace solver {
using enum model::CellState;
using model::CellState;
using model::Coord;
using model::SingleMove;
using model::VisitStatus;

constexpr int MAX_SOLVE_STEPS = 10000;

bool speculate_iterate(SpeculationContext & context);

// SpeculationResult
int
speculate_impl(SpeculationContext & context) {
  return 0;
}

Solution::ContextCache &
init_speculation_contexts(Solution & solution) {

  Solution::ContextCache & context_cache = solution.get_context_cache();
  context_cache.contexts.clear();
  context_cache.active_contexts.clear();
  context_cache.contradicting_contexts.clear();

  solution.board().visit_empty([&](Coord coord, CellState cell) {
    for (CellState state : {CellState::Bulb, CellState::Mark}) {
      auto & context = context_cache.contexts.emplace_back(
          0,
          solution.board(),
          SingleMove{
              model::Action::Add, CellState::Empty, CellState::Bulb, coord});

      context.board.apply_move(context.first_move);

      if (context.board.is_solved()) {
        context_cache.solved_contexts.push_back(&context);
      }
      else if (context.board.has_error()) {
        context_cache.contradicting_contexts.push_back(&context);
      }
      else {
        context_cache.active_contexts.push_back(&context);
      }
    }
  });
  return context_cache;
}

// returns depth of solution (approx some indicator of difficulty) or 0 if not
// found
size_t
speculate(Solution & solution) {
  assert(solution.is_solved() == false);
  assert(solution.has_error() == false);

  // creates N child boards with a different speculative move applied to each.
  Solution::ContextCache & cache = init_speculation_contexts(solution);
  auto & [_, active, contradictions, solved, forced] = cache;

  auto remove_from_active = [&](auto active_iter) {
    // pop_back invalidates iterators to back, and to end.  So be mindful that
    // active_iter may be already at the back()
    if (*active_iter != active.back()) {
      *active_iter = active.back();
      active.pop_back();
      return active_iter;
    }
    else {
      active.pop_back();
      return active.end();
    }
  };

  std::size_t depth = 0;
  // apply one batch of forced moves to all boards until we learn something.
  while (not active.empty()) {
    depth++;
    for (auto iter = active.begin(); iter != active.end();) {
      SpeculationContext & context = **iter;
      forced.clear();
      find_trivial_moves(context.board.board(), forced);

      // no forced moves is a dead-end
      if (forced.empty()) {
        iter = remove_from_active(iter);
        continue;
      }

      // Apply all of this iteration's forced moves
      bool inc_iter = true;
      for (auto & move : forced) {
        context.board.apply_move(move.next_move);
        if (context.board.has_error()) {
          contradictions.push_back(*iter);
          (*iter)->decision_type = move.reason;
          (*iter)->ref_location  = move.reference_location;
          iter                   = remove_from_active(iter);
          inc_iter               = false;
          break;
        }
        else if (context.board.is_solved()) {
          solved.push_back(*iter);
          iter     = remove_from_active(iter);
          inc_iter = false;
          break;
        }
      }
      if (inc_iter) {
        ++iter;
      }
    }
  }

  // TODO: find the shortest contradiction first by building a graph and doing
  // shortest path on them.  But for now, just take them in order of discovery.
  if (not contradictions.empty()) {
    for (auto * contradiction : contradictions) {
      AnnotatedMove move{contradiction->first_move};
      move.next_move.to_ = (move.next_move.to_ == CellState::Bulb)
                             ? CellState::Mark
                             : CellState::Bulb;
      solution.enqueue_move(move);
    }
    return depth;
  }
  return 0;
}

bool
find_moves(Solution & solution) {
  AnnotatedMoves moves;
  if (OptCoord invalid_mark_location =
          find_trivial_moves(solution.board().board(), moves)) {
    LOG_DEBUG("Detected a mark that cannot be illuminated at {}\n",
              *invalid_mark_location);
    solution.set_status(SolutionStatus::Impossible);
    solution.set_has_error(true, DecisionType::MARK_CANNOT_BE_ILLUMINATED);
    return false;
  }
  for (auto & move : moves) {
    solution.enqueue_move(move);
  }
  if (not moves.empty()) {
    return true;
  }

  if (size_t depth = speculate(solution)) {
    std::cout << " ==> DEPTH=" << depth << "\n";
    return true;
  }

  solution.set_status(SolutionStatus::FailedFindingMove);
  solution.board().set_has_error(true,
                                 DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION);
  return false;
}

bool
play_moves(Solution & solution) {
  bool played = not solution.empty_queue();
  while (not solution.empty_queue()) {
    auto const & next_move = solution.front();
    LOG_DEBUG("Playing Move: {} [{}] {}\n",
              next_move.next_move,
              to_string(next_move.motive),
              to_string(next_move.reason));
    solution.apply_enqueued_next();

    std::cout << solution.board() << std::endl;
  }
  return played;
}

void
find_solution(Solution & solution) {
  do {
    solution.add_step();
    find_moves(solution);
    if (not play_moves(solution)) {
      break;
    }
  } while (solution.get_status() == SolutionStatus::Progressing &&
           solution.get_step_count() < MAX_SOLVE_STEPS);
}

Solution
solve(model::BasicBoard const &        board,
      std::optional<model::BasicBoard> known_solution) {
  Solution solution(board, known_solution);
  solution.set_status(SolutionStatus::Progressing);

  find_solution(solution);

  if (solution.get_status() == SolutionStatus::Progressing) {
    solution.set_status(SolutionStatus::Terminated);
  }

  return solution;
}

} // namespace solver
