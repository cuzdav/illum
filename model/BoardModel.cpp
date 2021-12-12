#include "BoardModel.hpp"
#include "Serialize.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace model {

using namespace std::literals;

BoardModel::BoardModel(std::unique_ptr<StateChangeHandler> handler)
    : handler_(std::move(handler)) {}

BoardModel::BoardModel() : BoardModel(nullptr) {}

bool
BoardModel::operator==(BoardModel const & other) const {
  return started_ == other.started_ && board_ == other.board_ &&
         moves_ == other.moves_;
}

std::ostream &
operator<<(std::ostream & os, BoardModel const & model) {
  os << "BoardModel{" << (model.handler_ ? "<*>" : "<NoHandler>")
     << ", started: " << std::boolalpha << model.started_ << ",\n  moves: [\n";

  for (SingleMove const & m : model.moves_) {
    os << "    " << m << "\n";
  }
  os << "  ],\n  " << model.board_ << "}";
  return os;
}

void
BoardModel::start_game() {
  if (not started_) {
    started_ = true;
    // Adds a marker into game histroy separating the level setup from
    // the player's moves.
    moves_.push_back(
        {Action::StartGame, CellState::Empty, CellState::Empty, Coord{-1, -1}});
    num_setup_moves_ = moves_.size();
    on_state_change(
        Action::StartGame, CellState::Empty, CellState::Empty, Coord{-1, -1});
  }
}

bool
BoardModel::undo() {
  if (not started_ or moves_.back().action_ == Action::StartGame) {
    return false;
  }

  auto last = moves_.back();
  moves_.pop_back();
  Action action = last.action_ == Action::Add ? Action::Remove : Action::Add;

  board_.set_cell(last.coord_, last.from_);

  // note: to/from intentionally reversed since this is undo.
  on_state_change(action, last.to_, last.from_, last.coord_);

  return true;
}

bool
BoardModel::restart_game() {
  if (not started_) {
    return false;
  }
  while (undo()) {
    // nothing
  }
  return true;
}

void
BoardModel::apply_move(Action    action,
                       CellState from,
                       CellState to,
                       Coord     coord) {
  if (not board_.is_initialized()) {
    throw std::runtime_error("Board uninitialized");
  }
  if (board_.set_cell(coord, to)) {
    moves_.push_back({action, from, to, coord});
    on_state_change(action, from, to, coord);
  }
  else {
    throw std::runtime_error("Invalid coordinate for apply_move");
  }
}

void
BoardModel::add(CellState to_state, Coord coord) {
  CellState orig_cell = board_.get_cell(coord);
  apply_move(Action::Add, orig_cell, to_state, coord);
}

void
BoardModel::apply(SingleMove move) {
  apply_move(move.action_, move.from_, move.to_, move.coord_);
}

void
BoardModel::remove(Coord coord) {
  if (not started_) {
    throw std::runtime_error("Cannot remove cells before game has started");
  }
  CellState orig_cell = board_.get_cell(coord);
  if (orig_cell == CellState::Bulb or orig_cell == CellState::Mark) {
    apply_move(Action::Remove, orig_cell, CellState::Empty, coord);
  }
}

void
BoardModel::set_state_change_handler(
    std::unique_ptr<StateChangeHandler> handler) {
  handler_ = std::move(handler);
}

void
BoardModel::reset_game(int height, int width) {
  started_ = false;
  moves_.clear();
  board_.reset(height, width);
  Coord coord{height, width};
  moves_.push_back(
      {Action::ResetGame, CellState::Empty, CellState::Empty, coord});
  on_state_change(Action::ResetGame, CellState::Empty, CellState::Empty, coord);
}

void
BoardModel::reset_game(BasicBoard const & initial_board,
                       ResetGamePolicy    reset_policy) {
  started_ = false;
  moves_.clear();
  Coord dims = {initial_board.height(), initial_board.width()};
  moves_.push_back(
      {Action::ResetGame, CellState::Empty, CellState::Empty, dims});
  on_state_change(Action::ResetGame, CellState::Empty, CellState::Empty, dims);

  board_.reset(initial_board.height(), initial_board.width());

  std::vector<SingleMove> deferred;

  initial_board.visit_board(
      [this, &deferred, reset_policy](Coord coord, CellState cell) {
        if (cell != CellState::Empty) {
          if (is_dynamic_entity(cell)) {
            if (reset_policy == ResetGamePolicy::COPY_PLAYER_MOVES) {
              deferred.push_back(
                  SingleMove{Action::Add, CellState::Empty, cell, coord});
            }
          }
          else {
            board_.set_cell(coord, cell);
            moves_.push_back({Action::Add, CellState::Empty, cell, coord});
          }
        }
      });
  start_game();
  for (auto move : deferred) {
    board_.set_cell(move.coord_, move.to_);
  }
}

void
BoardModel::on_state_change(Action    action,
                            CellState prev_state,
                            CellState to_state,
                            Coord     coord) {
  if (handler_) {
    handler_->on_state_change(action, prev_state, to_state, coord);
  }
}

} // namespace model
