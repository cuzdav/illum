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
  context_cache.active_context_idxs.clear();
  context_cache.contradicting_context_idxs.clear();

  solution.board().visit_empty([&](Coord coord, CellState cell) {
    for (CellState state : {CellState::BULB, CellState::MARK}) {

      size_t idx     = context_cache.contexts.size();
      auto & context = context_cache.contexts.emplace_back(
          0,
          solution.board(),
          SingleMove{model::Action::ADD, CellState::EMPTY, state, coord});

      context.board.apply_move(context.first_move);

      if (context.board.has_error()) {
        assert(&context_cache.contexts[idx] == &context);
        context_cache.contradicting_context_idxs.push_back(idx);
      }
      else {
        context_cache.active_context_idxs.push_back(idx);
      }
    }
  });
  return context_cache;
}

Solution::ContextCache::IndicesIter
remove_from_active(Solution::ContextCache::Indices &   active_context_idxs,
                   Solution::ContextCache::IndicesIter active_iter) {
  // pop_back invalidates iterators to back, and to end.  So be mindful that
  // active_iter may be already at the back()
  if (*active_iter != active_context_idxs.back()) {
    *active_iter = active_context_idxs.back();
    active_context_idxs.pop_back();
    return active_iter;
  }
  else {
    active_context_idxs.pop_back();
    return active_context_idxs.end();
  }
};

// returns depth of solution (approx some indicator of difficulty) or 0 if not
// found
size_t
speculate(Solution & solution) {
  if (solution.is_solved()) {
    return 0;
  }

  assert(solution.has_error() == false);

  // creates N child boards with a different speculative move applied to
  // each.
  Solution::ContextCache & cache = init_speculation_contexts(solution);
  auto & [contexts, active_, contradictions, forced] = cache;

  // clang does not allow references to local bindings yet, (so "active_" above
  // cannot be used in lambda capture)
  auto & active = cache.active_context_idxs;

  std::size_t depth = 1;
  //  apply one batch of forced moves to all boards until we learn something.
  while (not active.empty()) {
    depth++;
    for (auto iter = active.begin(); iter != active.end();) {
      SpeculationContext & context = contexts[*iter];
      forced.clear();
      if (OptCoord unlightable_mark = find_trivial_moves(
              context.board.board(), solution.get_board_analysis(), forced)) {
        contradictions.push_back(*iter);
        SpeculationContext & context = contexts[(*iter)];
        context.decision_type        = DecisionType::MARK_CANNOT_BE_ILLUMINATED;
        context.ref_location         = *unlightable_mark;
        iter                         = remove_from_active(active, iter);
        continue;
      }

      // ////////////////////////////////// REMOVE
      // std::cout << "Speculating on: " << context.board.board() << "\n";
      // for (auto & debug_item : forced) {
      //   std::cout << "\tforced: " << debug_item << std::endl;
      // }
      ////////////////////////////////// END REMOVE

      // no forced moves is a dead-end
      if (forced.empty()) {
        iter = remove_from_active(active, iter);
        std::cout << "(DEAD END)\n";
        continue;
      }

      // Apply all of this iteration's forced moves
      bool inc_iter = true;
      for (auto & move : forced) {
        context.board.apply_move(move.next_move);
        if (context.board.has_error()) {
          std::cout << "(CONTRADICTION!)\n";
          contradictions.push_back(*iter);
          SpeculationContext & context = contexts[(*iter)];
          context.decision_type        = move.reason;
          context.ref_location         = move.reference_location;
          iter                         = remove_from_active(active, iter);
          inc_iter                     = false;
          break;
        }
        else if (context.board.is_solved()) {
          iter = remove_from_active(active, iter);
          std::cout << "(SOLVED)\n";
          inc_iter = false;
          break;
        }
      }
      if (inc_iter) {
        ++iter;
      }
    }
  }

  if (not contradictions.empty()) {
    for (int contradiction_idx : contradictions) {
      auto const &  contradiction = contexts[contradiction_idx];
      AnnotatedMove move{contradiction.first_move};
      move.next_move.to_      = (move.next_move.to_ == CellState::BULB)
                                  ? CellState::MARK
                                  : CellState::BULB;
      move.reason             = contradiction.decision_type;
      move.reference_location = contradiction.ref_location;
      solution.enqueue_move(move);
    }
    return depth;
  }
  return 0;
}

bool
find_moves(Solution & solution) {
  AnnotatedMoves moves;
  if (OptCoord invalid_mark_location = find_trivial_moves(
          solution.board().board(), solution.get_board_analysis(), moves)) {
    LOG_DEBUG("Detected a mark that cannot be illuminated at {}\n",
              *invalid_mark_location);
    solution.set_status(SolutionStatus::IMPOSSIBLE);
    solution.set_has_error(
        true, DecisionType::MARK_CANNOT_BE_ILLUMINATED, *invalid_mark_location);
    return false;
  }
  for (auto & move : moves) {
    solution.enqueue_move(move);
  }
  if (not moves.empty()) {
    return true;
  }

  if (size_t depth = speculate(solution)) {
    return true;
  }

  if (solution.has_error()) {
    return false;
  }

  solution.set_status(SolutionStatus::FailedFindingMove);
  solution.board().set_has_error(
      true, DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION, Coord{0, 0});
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
  } while (solution.get_status() == SolutionStatus::PROGRESSING &&
           solution.get_step_count() < MAX_SOLVE_STEPS);

  if (solution.has_error()) {
    OptCoord opt_coord = solution.board().get_ref_location();
    LOG_DEBUG("Board contains an error: {}, at: {}\n",
              solution.board().decision_type(),
              opt_coord);
  }
}

Solution
solve(model::BasicBoard const &        board,
      std::optional<model::BasicBoard> known_solution) {
  Solution solution(board, known_solution);
  if (solution.is_solved()) {
    solution.set_status(SolutionStatus::SOLVED);
    return solution;
  }
  if (solution.has_error()) {
    solution.set_status(SolutionStatus::IMPOSSIBLE);
    return solution;
  }
  solution.set_status(SolutionStatus::PROGRESSING);

  find_solution(solution);

  if (solution.get_status() == SolutionStatus::PROGRESSING) {
    solution.set_status(SolutionStatus::Terminated);
  }

  return solution;
}

} // namespace solver
