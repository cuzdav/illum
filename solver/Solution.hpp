#pragma once

#include "BasicBoard.hpp"
#include "Coord.hpp"
#include "PositionBoard.hpp"
#include "SingleMove.hpp"
#include "utils/DebugLog.hpp"
#include "utils/EnumUtils.hpp"
#include <iostream>
#include <optional>
#include <queue>
#include <stdexcept>

namespace solver {

enum class SolutionStatus {
  Initial,
  Progressing,
  Solved,
  Impossible,
  Ambiguous,

  Terminated,
  FailedFindingMove,
};

constexpr char const *
to_string(SolutionStatus status) {
  using enum SolutionStatus;
  switch (status) {
    case Initial:
      return "Initial";
    case Progressing:
      return "Progressing";
    case Solved:
      return "Solved";
    case Impossible:
      return "Impossible";
    case Ambiguous:
      return "Ambiguous";
    case Terminated:
      return "Terminated";
    case FailedFindingMove:
      return "FailedFindingMove";
  }
  return "<Unhandled SolutionStatus>";
}

enum class MoveMotive { FORCED, FOLLOWUP, SPECULATION };

constexpr char const *
to_string(MoveMotive const & mm) {
  using enum MoveMotive;
  switch (mm) {
    case FORCED:
      return "FORCED";
    case FOLLOWUP:
      return "FOLLOWUP";
    case SPECULATION:
      return "SPECULATION";
  }
  throw std::runtime_error("Unhandled MoveMotive");
}

struct AnnotatedMove {
  model::SingleMove next_move;
  DecisionType      reason;
  MoveMotive        motive;
  model::OptCoord   reference_location;

  friend auto operator<=>(AnnotatedMove const &,
                          AnnotatedMove const &) = default;
};

using OptAnnotatedMove = std::optional<AnnotatedMove>;

inline std::ostream &
operator<<(std::ostream & os, AnnotatedMove const & solution_move) {
  os << "{Moving: " << solution_move.next_move << " ==> "
     << to_string(solution_move.reason);
  if (solution_move.reference_location.has_value()) {
    os << " [In reference to " << *solution_move.reference_location << "]";
  }
  os << "}";
  return os;
}

class Solution {
public:
  using OptBoard = std::optional<model::BasicBoard>;

  Solution(PositionBoard const & board, OptBoard known_solution = std::nullopt)
      : board_(board), known_solution_(known_solution) {
    // nothing
  }

  void
  enqueue_move(AnnotatedMove next_move) {
    next_moves_.push(next_move);
  }

  void
  enqueue_bulb(model::Coord    coord,
               DecisionType    reason,
               MoveMotive      motive,
               model::OptCoord ref_coord = std::nullopt) {
    enqueue_move({model::SingleMove{model::Action::Add,
                                    board_.board().get_cell(coord),
                                    model::CellState::Bulb,
                                    coord},
                  reason,
                  motive,
                  ref_coord});
  }

  void
  enqueue_mark(model::Coord    coord,
               DecisionType    reason,
               MoveMotive      motive,
               model::OptCoord ref_coord = std::nullopt) {
    enqueue_move({model::SingleMove{model::Action::Add,
                                    board_.board().get_cell(coord),
                                    model::CellState::Mark,
                                    coord},
                  reason,
                  motive,
                  ref_coord});
  }

  bool
  apply_enqueued_next() {
    if (not next_moves_.empty()) {
      board_.apply_move(next_moves_.front().next_move);
      next_moves_.pop();
      return true;
    }
    return false;
  }

  bool
  apply_all_enqueued() {
    bool result = not next_moves_.empty();
    do {
    } while (apply_enqueued_next());
    return result;
  }

  SolutionStatus
  get_status() const {
    return status_;
  }

  bool
  is_solved() const {
    return board_.is_solved();
  }

  bool
  has_error() const {
    return board_.has_error();
  }

  void
  set_has_error(bool has_error) {
    board_.set_has_error(has_error);
  }

  PositionBoard const &
  board() const {
    return board_;
  }

  PositionBoard &
  board() {
    return board_;
  }

  void
  set_status(SolutionStatus status) {
    status_ = status;
  }

  int
  get_step_count() const {
    return step_count_;
  }

  void
  add_step() {
    step_count_++;
  }

  bool
  empty_queue() const {
    return next_moves_.empty();
  }

  auto const &
  front() const {
    return next_moves_.front();
  }

  void
  pop() {
    next_moves_.pop();
  }

private:
  PositionBoard             board_;
  OptBoard                  known_solution_;
  std::queue<AnnotatedMove> next_moves_;
  SolutionStatus            status_     = SolutionStatus::Initial;
  int                       step_count_ = 0;
};
} // namespace solver

DECLARE_FORMATTER(::solver::MoveMotive);
DECLARE_FORMATTER(::solver::SolutionStatus);
