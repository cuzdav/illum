#include "Solver.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "DebugLog.hpp"
#include "Direction.hpp"
#include "PositionBoard.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include "scope.hpp"
#include "trivial/trivial_moves.hpp"
#include <algorithm>
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

// Speculation involves "trying" to place a bulb (or mark) in each empty cell,
// and playing out the trivial/forced moves as a result to see if it causes a
// contradiction. We do a breadth-first search, to avoid finding complex move
// sequences before simple ones. This way, as soon as we find a contradiction,
// we know it's the simplest move (or is tied for simplest) and take it.

// if a sequence solves the game, we keep searching to see if there is also
// another solution, which means this path is invalid due to ambiguous results.

// NOTE: a tree of unique ptrs is stupid, but without the indirection it crashes
// clangd (currently version 14).  Holding SpeculationContext directly
// immediately kills it, guessing due to incomplete type.)
struct SpeculationContext {
  enum Status { HATCHED, STILL_SPECULATING, DEADEND, SOLVED, CONTRADICTION };
  using SpeculationContextUPtr = std::unique_ptr<SpeculationContext>;
  using ChildPaths = std::unordered_map<Coord, SpeculationContextUPtr>;

  int                       depth;
  Solution *                solution;
  PositionBoard             board;
  std::optional<SingleMove> my_move;
  Status                    status;
  std::vector<SingleMove>   unexplored_forced_moves;
  ChildPaths                child_paths;
};

// SpeculationResult
int
speculate_impl(SpeculationContext & context) {
  return 0;
}
//   if (context.board.is_solved()) {
//     retval = {context.my_move, context.depth,
//     SpeculationResult::HAS_SOLUTION}; if (++found_solution_count > 1) {
//       retval = {context.my_move,
//                 context.depth,
//                 SpeculationResult::FOUND_CONTRADICTION};
//       return model::STOP_VISITING;
//     }
//     else {
//       // first solution so far...
//       top_level_moves.push_back(context);
//       return model::KEEP_VISITING;
//     }
//   }
//   else if (context.board.has_error()) {
//     // already done!
//     retval = {
//         context.my_move, context.depth,
//         SpeculationResult::FOUND_CONTRADICTION};
//     return model::STOP_VISITING;
//   }

//   // Ok not done, does this position have more investigation necessary?
//   // If placing the bulb left some forcing moves, then we must keep going.
//   // Otherwise, forget this move it.
//   find_trivial_moves()
// });

// auto &       position_board = solution.board_;
// auto const & basic_board    = position_board.board();

// SpeculationContext context{
//     .depth = 0, .solution = solution, .board = position_board};

// if (spec_context.unexplored_forced_moves.empty()) {
//   LOG_DEBUG("{:>{}}[Depth {:>2}] Speculation begin\n",
//             spec_context.depth,
//             " ",
//             spec_context.depth);
//   // initialize our state since we are just starting out
//   basic_board.visit_empty([&](Coord coord, CellState cell) {

//   });
// }

// Solution::OptBoard one_solution;
// bool               played_move = false;
// auto &             basic_board = solution.board_.board();
//  basic_board.visit_empty([&](Coord coord, CellState) {
// Speculative move
// solution.step_count_++;
// solution.board_.clone_position();

//   LOG_DEBUG("[SPECULATE-BULB] Playing ({},{})\n", coord.row_,
//   coord.col_); solution.board_.add_bulb(coord);

//   if (solution.board_.has_error()) {

//     // This is actually success - we found a contradiction with this
//     // move
//     LOG_DEBUG(
//         "[SPECULATE-BULB] Contradiction, so  ({},{}) must be a "
//         "mark\n",
//         coord.row_,
//         coord.col_);
//     solution.board_.pop();
//     solution.board_.add_mark(coord);
//     played_move = true;
//     return model::STOP_VISITING;
//   }
//   else if (solution.board_.is_solved()) {
//     if (not one_solution.has_value()) {
//       one_solution.emplace(solution.board_.board());
//       solution.board_.pop();
//       // keep visiting to see if there are no more solutions
//       // if we finish and this is the only one, that's success.
//       return model::KEEP_VISITING;
//     }
//     else if (solution.board_.board() != *one_solution) {
//       // multiple different solutions?  Uh oh.
//       solution.board_.pop();
//       solution.status_ = SolutionStatus::Ambiguous;
//       return model::STOP_VISITING;
//     }
//   }
//   solution.board_.pop();
//   return model::KEEP_VISITING;
// });
// LOG_DEBUG("Done visiting empty spaces (speculate) steps={} {}\n",
//           solution.step_count_,
//           to_string(solution.status_));
// if (one_solution.has_value() &&
//     solution.status_ == SolutionStatus::Progressing) {
//   solution.status_         = SolutionStatus::Solved;
//   solution.known_solution_ = *one_solution;
//   return true;
// }
// return played_move;

struct SpeculationAnalysis {
  int                  num_solutions = 0;
  SpeculationContext * contradiction = nullptr;
};

using SpeculationContexts = std::vector<SpeculationContext>;

SpeculationAnalysis
speculation_prepare(SpeculationContexts & contexts) {
  std::erase_if(contexts, [](auto & context) {
    return context.status == SpeculationContext::DEADEND;
  });

  SpeculationAnalysis analysis;
  for (SpeculationContext & context : contexts) {
    if (context.board.has_error()) {
      analysis.contradiction = &context;
      break;
    }
    if (context.board.is_solved()) {
      ++analysis.num_solutions;
    }
  }
  return analysis;
}

void
handle_contradiction(SpeculationContext & context, Solution & solution) {
  Coord coord = context.my_move->coord_;
  LOG_DEBUG("[SPECULATE-BULB] Contradiction, so {} must be a mark\n", coord);
  if (context.my_move->to_ == CellState::Bulb) {
    // if bulb caused contradiction, it must be a mark
    solution.board_.add_mark(coord);
  }
  else {
    // if mark caused contradiction, it must be a bulb
    solution.board_.add_bulb(coord);
  }
}

SpeculationContexts
init_root_speculation_contexts(Solution & solution) {
  SpeculationContexts speculation_roots;
  speculation_roots.reserve(solution.board_.board().width() *
                            solution.board_.board().height());

  // NOTE: just-hatched contexts do NOT have the move applied to their board yet
  solution.board_.board().visit_empty([&](Coord coord, CellState cell) {
    speculation_roots.emplace_back(
        0,
        &solution,
        solution.board_.board(),
        SingleMove{
            model::Action::Add, CellState::Empty, CellState::Bulb, coord},
        SpeculationContext::HATCHED);
  });
  return speculation_roots;
}

bool
check_after_spec_move(SpeculationContext & context) {
  if (context.board.is_solved()) {
    context.status = SpeculationContext::SOLVED;
    return false;
  }
  else if (context.board.has_error()) {
    context.status = SpeculationContext::CONTRADICTION;
    return true;
  }
  else if (find_trivial_moves(context.board.board(),
                              context.unexplored_forced_moves)) {
    if (context.unexplored_forced_moves.empty()) {
      context.status = SpeculationContext::DEADEND;
      return false;
    }
    else {
      for (auto & move : context.unexplored_forced_moves) {
        context.child_paths.insert(std::pair{
            move.coord_,
            std::make_unique<SpeculationContext>(context.depth + 1,
                                                 context.solution,
                                                 context.board,
                                                 move,
                                                 SpeculationContext::HATCHED)});
      }
    }
  }
  return false;
}

// do the recursion...
void
speculate_iterate(SpeculationContext & context, Solution & solution) {
  if (context.status == SpeculationContext::HATCHED) {
    context.board.apply_move(*context.my_move);
    if (check_after_spec_move(context)) {
      // todo
    }
  }
}

bool
speculate(Solution & solution) {
  SpeculationContexts speculation_roots =
      init_root_speculation_contexts(solution);

  while (true) {
    SpeculationAnalysis analysis = speculation_prepare(speculation_roots);
    if (analysis.contradiction != nullptr) {
      auto & context = *analysis.contradiction;
      if (context.my_move) {
        handle_contradiction(context, solution);
        return true;
      }
      else {
        solution.status_ = SolutionStatus::Impossible;
        return false;
      }
    }
    if (speculation_roots.empty()) {
      // all deadends
      return false;
    }
    if (analysis.num_solutions > 1) {
      // too many solutions; invalid position
      solution.status_ = SolutionStatus::Ambiguous;
      return false;
    }

    for (auto & context : speculation_roots) {
      speculate_iterate(context, solution);
    }
  }
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
  if (play_any_forced_move(solution)) {
    return true;
  }

  if (speculate(solution)) {
    return true;
  }

  solution.status_ = SolutionStatus::FailedFindingMove;
  return false;
}

bool
play_single_move(Solution & solution) {
  if (solution.board_.is_solved()) {
    LOG_DEBUG("play_single_move: Solved! steps={}\n", solution.step_count_);
    solution.status_         = SolutionStatus::Solved;
    solution.known_solution_ = solution.board_.board();
    return true;
  }
  else if (solution.board_.has_error()) {
    LOG_DEBUG("play_single_move: ERROR! steps={}\n", solution.step_count_);
    solution.status_ = SolutionStatus::Impossible;
    return true;
  }
  else {
    return play_move(solution);
  }
  return false;
}

void
find_solution(Solution & solution) {
  do {
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
