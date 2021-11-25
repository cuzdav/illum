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
#include "trivial/trivial_moves.hpp"
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
// clangd (currently version 14).  Holding SpeculationContext directly
// immediately kills it, guessing due to incomplete type.)
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
  size_t               num_solutions         = 0;
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

  SpeculationAnalysis analysis;
  for (SpeculationContext & context : contexts) {
    switch (context.status) {
      case SpeculationContext::CONTRADICTION:
        analysis.contradiction = &context;
        return analysis;

      case SpeculationContext::SOLVED:
        ++analysis.num_solutions;
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
        "[CONTRADICTION] Bulb at {} caused a contradiction; must be a "
        "mark\n",
        move.next_move.coord_);
    solution.enqueue_mark(move.next_move.coord_,
                          move.reason,
                          MoveMotive::SPECULATION,
                          context.board.get_ref_location());
  }
  else {
    // if mark caused contradiction, it must be a bulb
    solution.enqueue_bulb(move.next_move.coord_,
                          move.reason,
                          MoveMotive::SPECULATION,
                          context.board.get_ref_location());
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

bool
apply_hatched_move(SpeculationContext & context) {
  assert(context.annotated_move.has_value());

  context.board.apply_move(context.annotated_move->next_move);
  // std::cout << "//\n// apply_hatched_move(SpeculationContext & context)\n// "
  //           << context.annotated_move->next_move << "\n//" << context.board
  //           << std::endl;

  if (context.board.is_solved()) {
    context.status = SpeculationContext::SOLVED;
    return true;
  }
  else if (context.board.has_error()) {
    context.status = SpeculationContext::CONTRADICTION;
    return true;
  }
  else if (not find_trivial_moves(context.board.board(),
                                  context.unexplored_forced_moves)) {
    // false does not mean it failed to find moves, but that it found a mark
    // that is not illuminable
    context.status = SpeculationContext::CONTRADICTION;
    return true;
  }

  /////////////////////////////////////////
  // DEBUG
  // std::cout << "find_trivial_moves() Size of moves is: "
  //           << context.unexplored_forced_moves.size() << "\n"
  //           << context.board << "\n";
  // for (auto & m : context.unexplored_forced_moves) {
  //   std::cout << m << std::endl;
  // }
  /////////////////////////////////////////

  if (context.unexplored_forced_moves.empty()) {
    //    std::cout << "Deadend\n";
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
  //  std::cout << "Speculate into children:\n";
  // for (auto & [child_coord, child_context] : context.child_paths) {
  //   std::cout << child_context << " "
  //             << (child_context ? to_string(child_context->status) : "")
  //             << std::endl;
  // }

  PositionBoard * solution_board = nullptr;

  int solved_count = 0;
  for (auto & [child_coord, child_context] : context.child_paths) {
    if (child_context) {
      speculate_iterate(*child_context);
      switch (child_context->status) {
        case SpeculationContext::STILL_SPECULATING:
          break;
        case SpeculationContext::DEADEND:
          context.child_paths.erase(child_coord);
          break;
        case SpeculationContext::SOLVED:
          ++solved_count;
          break;
        case SpeculationContext::CONTRADICTION:
          context.status = SpeculationContext::CONTRADICTION;
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
  std::cout << "***\n";
  std::cout << "*** TOP OF SPECULATE \n";
  std::cout << "***\n";
  std::cout << solution.board();

  SpeculationContexts speculation_roots =
      init_root_speculation_contexts(solution);

  assert(solution.is_solved() == false);
  // if (solution.has_error()) {
  //   std::cout << "Board has error: " << solution.board() << std::endl;
  // }
  assert(solution.has_error() == false);

  bool keep_going = true;
  while (keep_going) {
    // std::cout << "--- top of speculate infinite loop: numroots: "
    //           << speculation_roots.size() << "\n";

    SpeculationAnalysis analysis = speculation_setup(speculation_roots);

    // std::cout << "--- after setup, numroots: " << speculation_roots.size()
    //           << "\n";

    if (speculation_roots.empty()) {
      // all deadends
      //      std::cout << "--- all children are deadends\n";
      solution.set_status(SolutionStatus::Impossible);
      return false;
    }

    ///////////////////////////////// DEBUG BEGIN

    // std::cout << "num children: " << speculation_roots.size() << "\n";
    // std::cout << "analysis: contradiction: " <<
    // (bool)analysis.contradiction
    //           << "\n";
    // std::cout << "          num solutions:" << analysis.num_solutions <<
    // "\n"; int dbg = 0; for (auto & ctx : speculation_roots) {
    //   std::cout << dbg++ << ": state: " << ctx.status << "\n";
    //   std::cout << "has num_children:" << ctx.child_paths.size() << "\n";
    //   std::cout << "board: " << ctx.board << "\n";
    // }
    ///////////////////////////////// DEBUG END

    if (analysis.contradiction != nullptr) {
      auto & context = *analysis.contradiction;
      if (context.annotated_move) {
        // if my child is a contradiction, its move is invalid and can be
        // recorded as what not to do
        handle_contradiction(context, solution);
        return true;
      }
      else {
        // contradiction in child did not involve a move, so *I* am invalid
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
  bool board_is_valid = find_trivial_moves(solution.board().board(), moves);
  for (auto & move : moves) {
    solution.enqueue_move(move);
  }
  if (moves.empty()) {
    if (speculate(solution)) {
      return true;
    }
  }

  solution.set_status(SolutionStatus::FailedFindingMove);
  return false;
}

bool
play_moves(Solution & solution) {
  bool played = not solution.empty_queue();
  ;
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
