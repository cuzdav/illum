#pragma once

#include "BoardModel.hpp"

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
  SolutionStatus    status_      = SolutionStatus::Initial;
  int               step_count_  = 0;
  int               error_count_ = 0;
  model::BoardModel model_;
};

} // namespace solver
