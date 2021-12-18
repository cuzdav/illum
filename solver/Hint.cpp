#include "Hint.hpp"
#include "AnnotatedMove.hpp"
#include "DecisionType.hpp"
#include "Solver.hpp"

namespace solver {

std::ostream &
operator<<(std::ostream & os, Hint const & hint) {
  fmt::print(os, "{}", hint);
  return os;
}

Hint
Hint::create(model::BasicBoard const & board) {
  Solution solution(board);
  if (solution.has_error()) {
    Hint result(solution.decision_type());
    result.has_error_ = true;
    return result;
  }

  size_t depth = find_moves(solution);

  if (solution.empty_queue()) {
    return Hint{solution.decision_type()};
  }

  Hint result{solution.decision_type()};
  result.next_moves_.add(solution.front());
  solution.pop();

  if (solution.decision_type() == DecisionType::NONE) {
    // Apply forced move, no contradiction to show.
    auto const & current = result.next_moves_[0];
    while (not solution.empty_queue()) {
      auto const & next = solution.front();
      if (is_clustered_decision_type(next.reason) &&
          (next.reason == current.reason) &&
          (next.reference_location == current.reference_location)) {
        result.next_moves_.add(solution.front());
      }
      solution.pop();
    }

    std::cout << "decision type none!  Now what? " << std::endl;
  }
  else {
    fmt::print("Got Here: Solution has reason: {}\n", solution.decision_type());
    Hint::ExplainStep const & contradiction_moves = result.next_moves();
    assert(contradiction_moves.size() == 1);

    // flip the move to show a contradiction, and then play it out.
    auto contradiction_move = contradiction_moves[0];
    contradiction_move.next_move.to_ =
        contradiction_move.next_move.to_ == model::CellState::BULB
            ? model::CellState::MARK
            : model::CellState::BULB;

    result.explain_steps_.emplace_back(solution.decision_type())
        .add(contradiction_move);

    // ensure this is the only move to play
    solution.clear_queue();
    solution.enqueue_move(contradiction_move);

    while (not solution.has_error()) {
      solution.apply_all_enqueued();

      find_moves(solution);
      OptCoord prev_location;
      if (not solution.empty_queue()) {
        prev_location = solution.front().reference_location;
        result.explain_steps_.emplace_back(solution.front().reason)
            .add(solution.front());
        solution.pop();

        while (not solution.empty_queue() && prev_location &&
               solution.front().reference_location == prev_location) {
          result.explain_steps_.back().add(solution.front());
          solution.pop();
        }
        solution.clear_queue();
        for (auto next_move : result.explain_steps_.back().moves()) {
          solution.enqueue_move(next_move);
        }
        solution.apply_all_enqueued();
      }
    }
  }
  return result;
}

} // namespace solver
