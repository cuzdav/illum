#include "BoardModel.hpp"
#include "Serialize.hpp"
#include <iostream>
#include <stdexcept>
#include <utility>

namespace model {

BoardModel::BoardModel(std::unique_ptr<StateChangeHandler> handler)
    : handler_(std::move(handler)) {}

bool
BoardModel::operator==(BoardModel const & other) const {
  return started_ == other.started_ && board_ == other.board_ &&
         moves_ == other.moves_;
}

std::ostream &
operator<<(std::ostream & os, BoardModel const & model) {
  os << "BoardModel<" << (model.handler_ ? "<*>" : "<NULL>")
     << ", started: " << std::boolalpha << model.started_ << ", moves: [";

  for (SingleMove const & m : model.moves_) {
    os << "\n\t" << m;
  }
  os << "], " << model.board_ << ">";
  return os;
}

void
BoardModel::start_game() {
  if (not started_) {
    started_ = true;
    // Adds a marker into game histroy separating the level setup from
    // the player's moves.
    moves_.push_back({Action::StartGame, CellState::Empty, -1, -1});
    on_state_change(Action::StartGame, CellState::Empty, -1, -1);
  }
}

void
BoardModel::undo() {
  if (started_ and moves_.back().action_ != Action::StartGame) {
    auto last = moves_.back();
    moves_.pop_back();
    Action action = last.action_ == Action::Add ? Action::Remove : Action::Add;
    on_state_change(action, last.state_, last.row_, last.col_);
  }
}

void
BoardModel::apply_move(Action action, CellState state, int row, int col) {
  if (not board_.is_initialized()) {
    throw std::runtime_error("Board uninitialized");
  }
  moves_.push_back({action, state, row, col});
  on_state_change(action, state, row, col);
}

void
BoardModel::add(CellState state, int row, int col) {
  if (board_.set_cell(row, col, state)) {
    apply_move(Action::Add, state, row, col);
  }
}

void
BoardModel::remove(int row, int col) {
  CellState orig_cell = board_.get_cell(row, col);
  if (orig_cell == CellState::Bulb or orig_cell == CellState::Mark) {
    board_.set_cell(row, col, CellState::Empty);
    apply_move(Action::Remove, orig_cell, row, col);
  }
}

void
BoardModel::reset_game(int height, int width) {
  started_ = false;
  moves_.clear();
  board_.reset(height, width);
  moves_.push_back({Action::ResetGame, CellState::Empty, height, width});
  on_state_change(Action::ResetGame, CellState::Empty, height, width);
}

void
BoardModel::on_state_change(Action action, CellState state, int row, int col) {
  if (handler_) {
    handler_->on_state_change(action, state, row, col);
  }
}

} // namespace model
