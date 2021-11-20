#pragma once

#include "BasicBoard.hpp"
#include "PositionBoard.hpp"
#include <optional>

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

struct Solution {
  using OptBoard = std::optional<model::BasicBoard>;

  Solution(model::BasicBoard const & board) : board_(board) {}
  OptBoard       known_solution_;
  SolutionStatus status_      = SolutionStatus::Initial;
  int            step_count_  = 0;
  int            error_count_ = 0;
  PositionBoard  board_;
};

} // namespace solver
