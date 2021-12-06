#pragma once

#include "AnnotatedMove.hpp"
#include "BasicBoard.hpp"
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "PositionBoard.hpp"
#include "SingleMove.hpp"
#include "SpeculationContext.hpp"
#include "trivial_moves.hpp"
#include "utils/DebugLog.hpp"
#include "utils/EnumUtils.hpp"
#include <iostream>
#include <optional>
#include <queue>
#include <stdexcept>
#include <vector>

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

class Solution {
public:
  using OptBoard = std::optional<model::BasicBoard>;

  Solution(PositionBoard const & board, OptBoard known_solution = std::nullopt)
      : board_(board)
      , known_solution_(known_solution)
      , board_analysis_(create_board_analysis(board.board())) {
    auto const size = board.width() * board.height();
    context_cache_.contexts.reserve(size);
    context_cache_.active_context_idxs.reserve(size);
    context_cache_.contradicting_context_idxs.reserve(size);
  }

  bool
  is_ambiguous() const {
    return board_.decision_type() ==
           DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION;
  }

  DecisionType
  decision_type() const {
    return board_.decision_type();
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
      auto & next = next_moves_.front();
      LOG_DEBUG("{} {} {} {}\n",
                next.next_move,
                next.reason,
                next.motive,
                next.reference_location
                    ? fmt::format("{}", *next.reference_location)
                    : "");
      auto const next_move = next.next_move;
      next_moves_.pop();
      if (is_empty(board_.get_cell(next_move.coord_))) {
        board_.apply_move(next_move);
        if (is_solved()) {
          status_ = SolutionStatus::Solved;
        }
        else if (has_error()) {
          status_ = SolutionStatus::Impossible;
        }
        return true;
      }
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
  set_has_error(bool         has_error,
                DecisionType decision_type,
                model::Coord location) {
    board_.set_has_error(has_error, decision_type, location);
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

  BoardAnalysis *
  get_board_analysis() {
    return board_analysis_.get();
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

  struct ContextCache {
    std::vector<SpeculationContext> contexts;
    std::vector<int>                active_context_idxs;
    std::vector<int>                contradicting_context_idxs;
    std::vector<AnnotatedMove>      forced_moves;
  };

  ContextCache &
  get_context_cache() {
    return context_cache_;
  }

private:
  PositionBoard                  board_;
  OptBoard                       known_solution_;
  std::queue<AnnotatedMove>      next_moves_;
  SolutionStatus                 status_     = SolutionStatus::Initial;
  int                            step_count_ = 0;
  ContextCache                   context_cache_;
  std::unique_ptr<BoardAnalysis> board_analysis_;
};
} // namespace solver
