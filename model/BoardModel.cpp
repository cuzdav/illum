#include "BoardModel.hpp"
#include "Serialize.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace model {

BoardModel::BoardModel(StateChangeHandler * handler) : handler_(handler) {
  if (handler_ == nullptr) {
    throw std::runtime_error("Invalid BoardModel state change handler");
  }
}

bool
BoardModel::operator==(BoardModel const & other) const {
  return started_ == other.started_ && height_ == other.height_ &&
         width_ == other.width_ && cells_ == other.cells_ &&
         moves_ == other.moves_;
}

std::ostream &
operator<<(std::ostream & os, BoardModel const & model) {
  os << "BoardModel<" << (model.handler_ ? "<*>" : "<NULL>")
     << ", started: " << std::boolalpha << model.started_
     << ", height: " << model.height_ << ", width: " << model.width_
     << ", moves: [";

  for (SingleMove const & m : model.moves_) {
    os << "\n\t" << m;
  }
  os << "], cells:\n\t";
  int i = model.width_;

  for (CellState state : model.cells_) {
    os << to_char(state);
    if (--i == 0) {
      os << "\n\t";
      i = model.width_;
    }
  }
  os << "]";
  return os;
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

CellState
BoardModel::get_cell_from_flat_idx(int idx) const {
  if (idx >= 0 && idx < size(cells_)) {
    return cells_[idx];
  }
  throw std::range_error("idx out of bounds: " + std::to_string(idx));
}

int
BoardModel::get_idx(int row, int col) const {
  if (int idx = row * width_ + col; idx < size(cells_)) {
    return idx;
  }
  throw std::range_error("invalid row + col combination (out of bounds)" +
                         std::to_string(row) + ':' + std::to_string(col));
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
  moves_.push_back({Action::ResetGame, CellState::Empty, height, width});
  handler_->onStateChange(Action::ResetGame, CellState::Empty, height, width);
}

} // namespace model
