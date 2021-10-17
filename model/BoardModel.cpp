#include "BoardModel.hpp"
#include <stdexcept>

namespace model {

BoardModel::BoardModel(BoardModel::StateChangeHandler * handler)
  : handler_(handler) {
  if (handler_ == nullptr) {
    throw std::runtime_error("Invalid BoardModel state change handler");
  }
}

void
BoardModel::start_game() {
  if (not started_) {
    started_ = true;
    // Adds a marker into game histroy separating the level setup from
    // the player's moves.
    moves_.push_back({Action::StartGame, CellState::Empty, -1, -1});
    handler_->onStateChange(Action::StartGame, CellState::Empty, -1, -1);
  }
}

void
BoardModel::undo() {
  if (started_ and moves_.back().action_ != Action::StartGame) {
    auto last = moves_.back();
    moves_.pop_back();
    Action action = last.action_ == Action::Add ? Action::Remove : Action::Add;
    handler_->onStateChange(action, last.state_, last.row_, last.col_);
  }
}

int
BoardModel::get_idx(int row, int col) const {
  if (int idx = row * width_ + col; idx < size(cells_)) {
    return idx;
  }
  throw std::range_error("invalid row+col combination");
}

void
BoardModel::apply_move(Action action, CellState state, int row, int col) {
  if (cells_.empty()) {
    throw std::runtime_error("Board uninitialized");
  }
  moves_.push_back({action, state, row, col});
  handler_->onStateChange(action, state, row, col);
}

void
BoardModel::add(CellState state, int row, int col) {
  auto idx = get_idx(row, col);
  if (cells_[idx] == CellState::Empty) {
    cells_[idx] = state;
    apply_move(Action::Add, state, row, col);
  }
}

void
BoardModel::remove(int row, int col) {
  auto      idx      = get_idx(row, col);
  CellState curState = cells_[idx];
  if (curState == CellState::Bulb or curState == CellState::Mark) {
    cells_[idx] = CellState::Empty;
    apply_move(Action::Remove, curState, row, col);
  }
}

void
BoardModel::reset_game(int height, int width) {
  started_ = false;
  moves_.clear();
  cells_.clear();
  cells_.resize(height * width, CellState::Empty);
  height_ = height;
  width_  = width;
  handler_->onStateChange(Action::ResetGame, CellState::Empty, height, width);
}

} // namespace model
