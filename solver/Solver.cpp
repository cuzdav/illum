#include "Solver.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "DecisionType.hpp"
#include "Direction.hpp"
#include "PositionBoard.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
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

// Speculation involves "trying" to place a bulb (or mark) in each empty cell,
// and playing out the trivial/forced moves as a result to see if it causes a
// contradiction. We do a breadth-first search, to avoid finding complex move
// sequences before simple ones. This way, as soon as we find a contradiction,
// we know it's the simplest move (or is tied for simplest) and take it.

// if a sequence solves the game, we keep searching to see if there is also
// another solution, which means this path is invalid due to ambiguous results.

// NOTE: a tree of unique ptrs is stupid, but without the indirection it crashes
// clangd (currently version 14), which ruins productivity. Holding
// SpeculationContext directly immediately kills it, I guessing due to
// the incomplete/recursive type.)
struct SpeculationContext {
  enum Status { HATCHED, STILL_SPECULATING, DEADEND, SOLVED, CONTRADICTION };
  using SpeculationContextUPtr = std::unique_ptr<SpeculationContext>;
  using ChildPaths = std::unordered_map<Coord, SpeculationContextUPtr>;

  int                          depth;
  Solution *                   solution;
  PositionBoard                board;
  std::optional<AnnotatedMove> annotated_move;
  Status                       status;
  AnnotatedMoves               unexplored_forced_moves;
  ChildPaths                   child_paths;
  DecisionType                 decision_type = DecisionType::NONE;
  OptCoord                     ref_location;
};

char const *
to_string(SpeculationContext::Status s) {
  switch (s) {
    case SpeculationContext::HATCHED:
      return "HATCHED";
    case SpeculationContext::STILL_SPECULATING:
      return "STILL_SPECULATING";
    case SpeculationContext::DEADEND:
      return "DEADEND";
    case SpeculationContext::SOLVED:
      return "SOLVED";
    case SpeculationContext::CONTRADICTION:
      return "CONTRADICTION";
    default:
      return "(unknown SpeculationContext::Status)";
  }
}

std::ostream &
operator<<(std::ostream & os, SpeculationContext::Status s) {
  return os << to_string(s);
}

std::ostream &
operator<<(std::ostream & os, SpeculationContext const & sc) {

  os << "Context:[\n\t"
        "depth:    "
     << sc.depth << "\n"
     << "solution: "
     << "<solution>\n"
     << "board:    " << sc.board << "next_move:  ";

  if (sc.annotated_move.has_value()) {
    os << *sc.annotated_move << "\n";
  }
  os << "status:   " << sc.status << "\n"
     << "<vector>: {redacted}\n"
     << "children: " << sc.child_paths.size() << "\n]";
  return os;
}

bool speculate_iterate(SpeculationContext & context);

// SpeculationResult
int
speculate_impl(SpeculationContext & context) {
  return 0;
}

struct SpeculationAnalysis {
  bool                 multiple_solutions    = false;
  size_t               num_still_speculating = 0;
  size_t               num_hatchlings        = 0;
  SpeculationContext * contradiction         = nullptr;
};

using SpeculationContexts = std::vector<SpeculationContext>;

SpeculationAnalysis
speculation_setup(SpeculationContexts & contexts) {
  std::erase_if(contexts, [](auto & context) {
    return context.status == SpeculationContext::DEADEND;
  });

  SpeculationContext * solution = nullptr;

  SpeculationAnalysis analysis;
  for (SpeculationContext & context : contexts) {
    switch (context.status) {
      case SpeculationContext::CONTRADICTION:
        analysis.contradiction = &context;
        return analysis;

      case SpeculationContext::SOLVED:
        if (solution == nullptr) {
          solution = &context;
        }
        else {
          analysis.multiple_solutions |= context.board != solution->board;
        }
        break;

      case SpeculationContext::STILL_SPECULATING:
        ++analysis.num_still_speculating;
        break;

      case SpeculationContext::HATCHED:
        ++analysis.num_hatchlings;
        break;
    }
  }
  return analysis;
}

void
handle_contradiction(SpeculationContext & context, Solution & solution) {
  assert(context.annotated_move.has_value());

  auto move = *context.annotated_move;
  if (move.next_move.to_ == CellState::Bulb) {
    LOG_DEBUG(
        "[CONTRADICTION] Bulb at {} caused a contradiction: {} at {}, so it "
        "must be a mark\n",
        move.next_move.coord_,
        context.decision_type,
        context.ref_location.value_or(move.next_move.coord_));
    solution.enqueue_mark(move.next_move.coord_,
                          context.decision_type,
                          MoveMotive::SPECULATION,
                          context.ref_location);
  }
  else {
    // if mark caused contradiction, it must be a bulb
    solution.enqueue_bulb(move.next_move.coord_,
                          context.decision_type,
                          MoveMotive::SPECULATION,
                          context.ref_location);
  }
}

SpeculationContexts
init_root_speculation_contexts(Solution & solution) {
  SpeculationContexts speculation_roots;
  speculation_roots.reserve(solution.board().width() *
                            solution.board().height());

  // NOTE: just-hatched contexts do NOT have the move applied to their
  // board yet

  // first speculate on bulbs
  solution.board().visit_empty([&](Coord coord, CellState cell) {
    speculation_roots.emplace_back(0,
                                   &solution,
                                   solution.board(),
                                   AnnotatedMove{SingleMove{model::Action::Add,
                                                            CellState::Empty,
                                                            CellState::Bulb,
                                                            coord},
                                                 DecisionType::SPECULATION,
                                                 MoveMotive::SPECULATION},
                                   SpeculationContext::HATCHED);
  });

  // then speculate on marks
  solution.board().visit_empty([&](Coord coord, CellState cell) {
    speculation_roots.emplace_back(0,
                                   &solution,
                                   solution.board(),
                                   AnnotatedMove{SingleMove{model::Action::Add,
                                                            CellState::Empty,
                                                            CellState::Mark,
                                                            coord},
                                                 DecisionType::SPECULATION,
                                                 MoveMotive::SPECULATION},
                                   SpeculationContext::HATCHED);
  });
  return speculation_roots;
}

// A hatched move is one that has not yet been played, but was setup to be
// played later. Since we are doing a breadth-first search, each node's
// children are added one iteration, and "hatch" the next iteration (and are
// applied). This way we go across the tree, rather than DFS, so we find the
// shortest path first to reach a contradiction.
bool
apply_hatched_move(SpeculationContext & context) {
  assert(context.annotated_move.has_value());

  context.board.apply_move(context.annotated_move->next_move);
  if (context.board.is_solved()) {
    context.status = SpeculationContext::SOLVED;
    return true;
  }
  else if (context.board.has_error()) {
    context.status        = SpeculationContext::CONTRADICTION;
    context.decision_type = context.board.decision_type();
    context.ref_location  = context.board.get_ref_location();
    return true;
  }
  else if (auto opt_unlightable_mark = find_trivial_moves(
               context.board.board(), context.unexplored_forced_moves)) {
    // false does not mean it failed to find moves, but that it found a mark
    // that is not illuminable
    context.status = SpeculationContext::CONTRADICTION;
    context.annotated_move->reference_location = *opt_unlightable_mark;
    return true;
  }

  if (context.unexplored_forced_moves.empty()) {
    context.status = SpeculationContext::DEADEND;
  }
  else {
    for (auto & move : context.unexplored_forced_moves) {
      context.child_paths.insert(std::pair{
          move.next_move.coord_,
          std::make_unique<SpeculationContext>(context.depth + 1,
                                               context.solution,
                                               context.board,
                                               move,
                                               SpeculationContext::HATCHED)});
    }
    context.unexplored_forced_moves.clear();
    context.status = SpeculationContext::STILL_SPECULATING;
  }

  return false;
}

void
speculate_into_children(SpeculationContext & context) {
  PositionBoard * solution_board = nullptr;

  int solved_count = 0;
  for (auto & [child_coord, child_context] : context.child_paths) {
    if (child_context) {
      speculate_iterate(*child_context);
      switch (child_context->status) {
        case SpeculationContext::STILL_SPECULATING:
          break;
        case SpeculationContext::SOLVED:
          ++solved_count;
          break;
        case SpeculationContext::CONTRADICTION:
          context.status        = SpeculationContext::CONTRADICTION;
          context.decision_type = child_context->decision_type;
          context.ref_location  = child_context->ref_location;
          break;

        case SpeculationContext::HATCHED:
          throw std::runtime_error(
              "BUG: Should never be hatched after iterating child.");
      }
    }
    if (context.status == SpeculationContext::CONTRADICTION) {
      break;
    }
  }

  std::erase_if(context.child_paths, [](auto const & coord_child) {
    return coord_child.second->status == SpeculationContext::DEADEND;
  });

  // if one of my children is a solution, then all of my children should be
  // (same) solution, and then I'm a solution.
  if ((solved_count > 0) && (solved_count == context.child_paths.size())) {
    context.status = SpeculationContext::SOLVED;
  }
}

// top level evaluation of a single speculation tree
bool
speculate_iterate(SpeculationContext & context) {
  switch (context.status) {
    case SpeculationContext::HATCHED:
      apply_hatched_move(context);
      break;

    case SpeculationContext::CONTRADICTION:
      return true;

    case SpeculationContext::STILL_SPECULATING:
      speculate_into_children(context);
      break;

    case SpeculationContext::SOLVED:
    case SpeculationContext::DEADEND:
      break;
  }
  return false;
}

bool
speculate(Solution & solution) {
  SpeculationContexts speculation_roots =
      init_root_speculation_contexts(solution);

  assert(solution.is_solved() == false);
  assert(solution.has_error() == false);

  bool keep_going = true;
  while (keep_going) {
    SpeculationAnalysis analysis = speculation_setup(speculation_roots);
    if (analysis.multiple_solutions) {
      solution.set_status(SolutionStatus::Ambiguous);
      solution.board().set_has_error(
          true, DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION);
      return true;
    }
    if (speculation_roots.empty()) {
      // all deadends
      solution.set_status(SolutionStatus::Impossible);
      return false;
    }
    if (analysis.contradiction != nullptr) {
      auto & context = *analysis.contradiction;
      if (context.annotated_move) {
        // if my child is a contradiction, its move is invalid and can be
        // recorded as what not to do
        handle_contradiction(context, solution);
        return true;
      }
      else {
        // contradiction in child did not involve a move, so *I* am (already)
        // invalid
        solution.set_status(SolutionStatus::Impossible);
        return false;
      }
    }

    keep_going = analysis.num_still_speculating + analysis.num_hatchlings > 0;
    if (keep_going) {
      for (auto & context : speculation_roots) {
        speculate_iterate(context);
      }
    }
  }
  return false;
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
  if (speculate(solution)) {
    return true;
  }

  solution.set_status(SolutionStatus::FailedFindingMove);
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

bool
check_if_done(Solution & solution) {
  if (solution.is_solved()) {
    LOG_DEBUG("play_single_move: Solved! steps={}\n",
              solution.get_step_count());
    solution.set_status(SolutionStatus::Solved);
    return true;
  }
  else if (solution.has_error()) {
    LOG_DEBUG("play_single_move: ERROR! steps={}\n", solution.get_step_count());
    solution.set_status(SolutionStatus::Impossible);
    return true;
  }
  return false;
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
