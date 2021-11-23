#include "AnalysisBoard.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "Direction.hpp"
#include "utils/DebugLog.hpp"

namespace solver {

AnalysisBoard::AnalysisBoard(model::BasicBoard const & current) {
  position_boards_.reserve(4);
  position_boards_.push_back(current);
}

PositionBoard &
AnalysisBoard::cur() {
  return position_boards_.back();
}

PositionBoard const &
AnalysisBoard::cur() const {
  return position_boards_.back();
}

void
AnalysisBoard::clone_position() {
  position_boards_.push_back(position_boards_.back());
}

void
AnalysisBoard::pop() {
  if (position_boards_.size() == 1) {
    return;
  }
  position_boards_.pop_back();
}

} // namespace solver
