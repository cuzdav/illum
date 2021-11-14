#pragma once

#include "AnalysisBoard.hpp"
#include "BasicBoard.hpp"

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

struct Solution {
  Solution(model::BasicBoard const & board) : board_(board) {}

  SolutionStatus status_      = SolutionStatus::Initial;
  int            step_count_  = 0;
  int            error_count_ = 0;
  AnalysisBoard  board_;
};

} // namespace solver
