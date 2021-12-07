#include "AnalysisBoard.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "Direction.hpp"
#include "utils/DebugLog.hpp"
#include <iostream>

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
  if (visit_depth_ == 0) {
    position_boards_.push_back(position_boards_.back());
  }
  else {
    throw std::runtime_error(
        "Cannot clone while visiting - may invalidate iterators");
  }
}

void
AnalysisBoard::pop() {
  if (position_boards_.size() == 1) {
    return;
  }
  if (visit_depth_ == 0) {
    position_boards_.pop_back();
  }
  else {
    throw std::runtime_error(
        "Cannot pop while visiting - will invalidate iterators");
  }
}

std::ostream &
operator<<(std::ostream & os, AnalysisBoard const & aboard) {
  os << "AnalysisBoard{Depth=" << aboard.position_boards_.size() << ", "
     << aboard.cur() << "}";
  return os;
}

} // namespace solver
